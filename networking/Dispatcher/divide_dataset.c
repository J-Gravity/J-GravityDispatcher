#include "dispatcher.h"
#include <math.h>
#include <limits.h>
#include "lz4.h"
#include "transpose.h"

#define SORT_NAME mort
#define SORT_TYPE t_sortbod
#define SORT_CMP(x, y) (sbod_comp(&x, &y))
#include "sort.h"

void print_bounds(t_bounds b)
{
    printf("bounds: %f %f %f %f %f %f\n", b.xmin, b.xmax, b.ymin, b.ymax, b.zmin, b.zmax);
}

void print_cl4(cl_float4 v)
{
    printf("cl4: %f %f %f %f\n", v.x, v.y, v.z, v.w);
}

// method to seperate bits from a given integer 3 positions apart
uint64_t splitBy3(const unsigned int a)
{
    uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
    x = (x | x << 32) & 0x1f00000000ffff;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
    x = (x | x << 16) & 0x1f0000ff0000ff;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
    x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    x = (x | x << 2) & 0x1249249249249249;
    return x;
}
 
uint64_t mortonEncode_magicbits(const unsigned int x, const unsigned int y, const unsigned int z)
{
	//interweave the rightmost 21 bits of 3 unsigned ints to generate a 63-bit morton code
    uint64_t answer = 0;
    answer |= splitBy3(z) | splitBy3(y) << 1 | splitBy3(x) << 2;
    return answer;
}

uint64_t morton64(float x, float y, float z)
{
	//x, y, z are in [0, 1]. multiply by 2^21 to get 21 bits, confine to [0, 2^21 - 1]
    x = fmin(fmax(x * 2097152.0f, 0.0f), 2097151.0f);
    y = fmin(fmax(y * 2097152.0f, 0.0f), 2097151.0f);
    z = fmin(fmax(z * 2097152.0f, 0.0f), 2097151.0f);
    return (mortonEncode_magicbits((unsigned int)x, (unsigned int)y, (unsigned int)z));
}

int body_in_bounds(t_body body, t_bounds bounds)
{
    if (body.position.x > bounds.xmax || body.position.x < bounds.xmin)
        return (0);
    if (body.position.y > bounds.ymax || body.position.y < bounds.ymin)
        return (0);
    if (body.position.z > bounds.zmax || body.position.z < bounds.zmin)
        return (0);
    return (1);
}

t_tree *new_tnode(t_body *bodies, int count, t_tree *parent)
{
    t_tree *node = (t_tree *)calloc(1, sizeof(t_tree));
    node->bodies = bodies;
    node->count = count;
    node->parent = parent;
    node->children = NULL;
    return (node);
}

int node_depth(t_tree *node)
{
    int depth = 0;
    while (node->parent)
    {
        node = node->parent;
        depth++;
    }
    return depth;
}

#define xmid parent.xmax - (parent.xmax - parent.xmin) / 2
#define ymid parent.ymax - (parent.ymax - parent.ymin) / 2
#define zmid parent.zmax - (parent.zmax - parent.zmin) / 2

t_bounds bounds_from_code(t_bounds parent, unsigned int code)
{
	//generate the appropriate subbounds of the parent cell for this 3-bit string
    t_bounds bounds;
    if(code >> 2)
    {
        bounds.xmin = xmid;
        bounds.xmax = parent.xmax;
    }
    else
    {
        bounds.xmin = parent.xmin;
        bounds.xmax = xmid;
    }
    code %= 4;
    if (code >> 1)
    {
        bounds.ymin = ymid;
        bounds.ymax = parent.ymax;
    }
    else
    {
        bounds.ymin = parent.ymin;
        bounds.ymax = ymid;
    }
    code %= 2;
    if (code)
    {
        bounds.zmin = zmid;
        bounds.zmax = parent.zmax;
    }
    else
    {
        bounds.zmin = parent.zmin;
        bounds.zmax = zmid;
    }
    return (bounds);
}

static cl_float4 vadd(cl_float4 a, cl_float4 b)
{
    //add two vectors.
    return ((cl_float4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w});
}

static cl_float4 center_add(cl_float4 total, cl_float4 add)
{
	//method for tallying running total for center of gravity
    add.w = fabs(add.w);
    add.x *= add.w;
    add.y *= add.w;
    add.z *= add.w;
    return (cl_float4){total.x + add.x, total.y + add.y, total.z + add.z, total.w + add.w};
}

static t_body COG_from_children(t_tree **children)
{
	cl_float4 center = (cl_float4){0,0,0,0};
    float real_total = 0;
    for (int i = 0; i < 8; i++)
    {
        center.x += children[i]->as_single->bodies[0].position.x * children[i]->as_single->bodies[0].velocity.w;
        center.y += children[i]->as_single->bodies[0].position.y * children[i]->as_single->bodies[0].velocity.w;
        center.z += children[i]->as_single->bodies[0].position.z * children[i]->as_single->bodies[0].velocity.w;
        center.w += children[i]->as_single->bodies[0].velocity.w;
        real_total += children[i]->as_single->bodies[0].position.w;
    }
    t_body b;
    b.velocity = (cl_float4){0, 0, 0, center.w};
    center.x /= center.w;
    center.y /= center.w;
    center.z /= center.w;
    center.w = real_total;
    b.position = center;
    return b;
}

static t_body COG_from_bodies(t_body *bodies, int count)
{
    cl_float4 center = (cl_float4){0,0,0,0};
    if (count == 0)
        return (t_body){center, center};
    float real_total = 0;
    for (int i = 0; i < count; i++)
    {
        center = center_add(center, bodies[i].position);
        real_total += bodies[i].position.w;
    }
    t_body b;
    b.velocity = (cl_float4){0, 0, 0, center.w};
    center.x /= center.w;
    center.y /= center.w;
    center.z /= center.w;
    center.w = real_total;
    b.position = center;
    return b;
}

static t_tree *make_as_single(t_tree *c)
{
	//it's silly to build a whole cell for something
	//that could easily just be a t_body in t_tree
    t_body b;
    if (!c->children)
	   b = COG_from_bodies(c->bodies, c->count);
    else
        b = COG_from_children(c->children);
    t_tree *s = calloc(1, sizeof(t_tree));
    s->parent = NULL;
    s->children = NULL;
    s->count = 1;
    s->as_single = NULL;
    s->bodies = calloc(1, sizeof(t_body));
    s->bodies[0] = b;
    return (s);
}

static int count_tree_array(t_tree **arr)
{
    int count;
     if (!arr)
        return 0;
    for (count = 0; arr[count]; count++)
        ;
    return (count);
 }

static t_tree **enumerate_leaves(t_tree *root)
{
    //return a linear t_tree** that's all the leaf nodes (ie childless nodes) in the tree
    //this is an excellent opportunity to very quickly do centers of gravity/as_single
    t_tree **ret;

    if (!root->children)
    {
        //root->as_single = make_as_single(root);
        ret = (t_tree **)calloc(2, sizeof(t_tree *));
        ret[0] = root->count ? root : NULL; //we do not bother enumerating empty leaves (empty cells cant have children so this covers all)
        ret[1] = NULL;
        return (ret);
    }
    t_tree ***returned = (t_tree ***)calloc(8, sizeof(t_tree **));
    int total = 0;
    for (int i = 0; i < 8; i++)
    {
        returned[i] = enumerate_leaves(root->children[i]);
        total += count_tree_array(returned[i]);
    }
    //root->as_single = make_as_single(root);
    ret = (t_tree **)calloc(total + 1, sizeof(t_tree *));
    for (int i = 0; i < total;)
    {
        for (int j = 0; j < 8; j++)
        {
            for (int k = 0; returned[j][k]; k++, i++)
            {
                ret[i] = returned[j][k];
            }
            free(returned[j]);
        }
        free(returned);
    }
    ret[total] = NULL;
    return (ret);
}

static cl_float4 midpoint_from_bounds(t_bounds b)
{
    return (cl_float4){(b.xmax - b.xmin) / 2, (b.ymax - b.ymin) / 2, (b.zmax - b.zmin) / 2};
}

static float multipole_acceptance_criterion(t_tree *us, t_tree *them)
{
    //assess whether a cell is "near" or "far" for the sake of barnes-hut
    //if the value returned is less than THETA, that cell is far
    float s;
    float d;
    cl_float4 r;
    cl_float4 us_midpoint;

    if (us == them)
        return (THETA);
    us_midpoint = midpoint_from_bounds(us->bounds);
    s = them->bounds.xmax - them->bounds.xmin;
    r.x = them->as_single->bodies[0].position.x - us_midpoint.x;
    r.y = them->as_single->bodies[0].position.y - us_midpoint.y;
    r.z = them->as_single->bodies[0].position.z - us_midpoint.z;
    d = sqrt(r.x * r.x + r.y * r.y + r.z * r.z);

    //in normal Barnes-Hut, the MAC is evaluated for every body vs every cell. 
    //we're evaluating it cell to cell for speed and to build good workunits.
    //this could result in some comparisons that should be near being done as far.
    //subtracting half of our cell's width from the measured distance compensates for this adequately.
    //basically, we are measuring as if all the bodies in our cell are right up against the nearest side of our cell.
    d -= (us->bounds.xmax - us->bounds.xmin) / 2;
    return (s/d);
}

static t_tree **assemble_neighborhood(t_tree *cell, t_tree *root)
{
    t_tree **ret;
    t_tree ***returned;

    /*
        recursively flow through the tree, determining if cells are near or far from
        the cell we're currently considering. We skip the root.
        
        If the cell is far away (m_a_c < THETA), that cell is far enough away to treat as 1 particle.
        we create a copy of it as 1 particle and add that to the neighborhood.
        
        if the cell is nearby and childless (ie leaf), it is near enough that direct calculation is necessary,
        so it returns a null-terminated array just containing a pointer to the cell.

        if the cell is nearby and has children, we recurse down to its children.
        we make space for the 8 arrays that will be returned (some might be null)
        then we copy them into one final array and return it.

        in this way, we enumerate the "Neighborhood" of the cell, ie the bodies we'll need to compare with on the GPU.
    */
    if (root->parent && multipole_acceptance_criterion(cell, root) < THETA)
    {
        ret = (t_tree **)calloc(2, sizeof(t_tree *));
        ret[0] = root->as_single;
        ret[1] = NULL;
        return (ret);
    }
    else if (!(root->children))
    {
        ret = (t_tree **)calloc(2, sizeof(t_tree *));
        ret[0] = root;
        ret[1] = NULL;
        return (ret);
    }
    else
    {
        returned = (t_tree ***)calloc(8, sizeof(t_tree **));
        int total = 0;
        for (int i = 0; i < 8; i++)
        {
            returned[i] = assemble_neighborhood(cell, root->children[i]);
            total += count_tree_array(returned[i]);
        }
        ret = (t_tree **)calloc(total + 1, sizeof(t_tree *));
        for (int i = 0; i < total;)
        {
            for (int j = 0; j < 8; j++)
            {
                if (!returned[j])
                    continue ;
                for (int k = 0; returned[j][k]; k++, i++)
                {
                    ret[i] = returned[j][k];
                }
                free(returned[j]);
            }
            free(returned);
        }
        ret[total] = NULL;
        return (ret);
    }
}

static t_bounds bounds_from_bodies(t_body *bodies, int count)
{
    //at the start of making the tree, we need a box that bounds all the bodies.
    float xmin = 0, xmax = 0;
    float ymin = 0, ymax = 0;
    float zmin = 0, zmax = 0;

    for (int i = 0; i < count; i++)
    {
        //expand bounds if needed
        if (bodies[i].position.x < xmin)
            xmin = bodies[i].position.x;
        if (bodies[i].position.x > xmax)
            xmax = bodies[i].position.x;

        if (bodies[i].position.y < ymin)
            ymin = bodies[i].position.y;
        if (bodies[i].position.y > ymax)
            ymax = bodies[i].position.y;

        if (bodies[i].position.z < zmin)
            zmin = bodies[i].position.z;
        if (bodies[i].position.z > zmax)
            zmax = bodies[i].position.z;
    }

    //bounds must be a cube.
    float min = xmin;
    float max = xmax;
    if (ymin < min)
        min = ymin;
    if (zmin < min)
        min = zmin;
    if (ymax > max)
        max = ymax;
    if (zmax > max)
        max = zmax;
    printf("bounds from bodies were %f %f %f %f %f %f\n", min, max, min, max, min, max);
    return ((t_bounds){min, max, min, max, min, max});
}

t_bundle *bundle_leaves(t_tree **leaves, int offset, int count)
{
    t_dict *dict = create_dict(1000); //fiddle with this number later. it doesnt need to be big.
    
    //take up to Count leaves starting at offset and pour their neighborhoods
    //into the double-key hash. track which leaves we put in.
    t_pair *ids = NULL;
	long difficulty = 0;
	long size = 0;
    for (int i = 0; i < count && leaves[offset + i]; i++)
    {
        if (!ids)
            ids = create_pair(i + offset);
        else
        {
            t_pair *p = create_pair(i + offset);
            p->next_key = ids;
            ids = p;
        }
		size += leaves[i]->count * sizeof(t_body);
        t_tree **adding = leaves[offset + i]->neighbors;
        for (int j = 0; adding[j]; j++)
		{
            dict_insert(dict, adding[j], i);
			difficulty += leaves[offset + i]->count * adding[j]->count;
			size += adding[j]->count * sizeof(cl_float4);
		}
    }
	t_bundle *b = bundle_dict(dict, ids);
	b->difficulty = difficulty;
	b->size = size;
    return (b);
}

t_msg compress_msg(t_msg m)
{
    t_msg c;

    char *transposed = calloc(1, m.size);
    tpenc((unsigned char *)m.data, m.size, (unsigned char *)transposed, sizeof(int));
    int max_compressed_size = LZ4_compressBound(m.size);
    char *compressed = calloc(1, max_compressed_size);
    int result_compressed_size = LZ4_compress_default(transposed, compressed, m.size, max_compressed_size);
    printf("compressed to %d from %zu, %.2f\n", result_compressed_size, m.size, (float)result_compressed_size * 100 / (float)m.size);
    c.data = calloc(1, result_compressed_size + sizeof(int));
    memcpy(c.data, &m.size, sizeof(int));
    memcpy(c.data + sizeof(int), compressed, result_compressed_size);
    c.size = result_compressed_size + sizeof(int);
    free(transposed);
    free(compressed);
    free(m.data);
    return c;
}

t_msg serialize_bundle(t_bundle *b, t_tree **leaves)
{
    t_msg m;
    m.size = sizeof(int) * 2;
    for (int i = 0; i < b->keycount; i++)
    {
        m.size += sizeof(int) * 2;
        m.size += leaves[b->keys[i]]->count * sizeof(t_body);
    }
    for (int i = 0; i < b->cellcount; i++)
    {
        m.size += sizeof(int) * 2;
        m.size += b->cells[i]->count * sizeof(cl_float4);
        m.size += b->matches_counts[i] * sizeof(int);
    }
    m.data = malloc(m.size);
    size_t offset = 0;
    memcpy(m.data + offset, &(b->keycount), sizeof(int));
    offset += sizeof(int);
    
    memcpy(m.data + offset, &(b->cellcount), sizeof(int));
    offset += sizeof(int);
    for (int i = 0; i < b->keycount; i++)
    {
        memcpy(m.data + offset, &(b->keys[i]), sizeof(int));
        offset += sizeof(int);
        memcpy(m.data + offset, &(leaves[b->keys[i]]->count), sizeof(int));
        offset += sizeof(int);
        memcpy(m.data + offset, leaves[b->keys[i]]->bodies, leaves[b->keys[i]]->count * sizeof(t_body));
        offset += leaves[b->keys[i]]->count * sizeof(t_body);
    }
    for (int i = 0; i < b->cellcount; i++)
    {
        memcpy(m.data + offset, &(b->matches_counts[i]), sizeof(int));
        offset += sizeof(int);
        memcpy(m.data + offset, b->matches[i], b->matches_counts[i] * sizeof(int));
        offset += b->matches_counts[i] * sizeof(int);
        memcpy(m.data + offset, &(b->cells[i]->count), sizeof(int));
        offset += sizeof(int);
        for (int j = 0; j < b->cells[i]->count; j++)
        {
            memcpy(m.data + offset, &(b->cells[i]->bodies[j].position), sizeof(cl_float4));
            offset += sizeof(cl_float4);
        }
    }
    printf("bundle size final was %.1fMB\n", (float)m.size / 1048576.0f);
    return (m);
}

t_sortbod *make_sortbods(t_body *bodies, t_bounds bounds, int count)
{
    t_sortbod *sorts = calloc(count, sizeof(t_sortbod));
    float distance =  1.0 / (bounds.xmax - bounds.xmin);
    for (int i = 0; i < count; i++)
    {
        sorts[i].morton = morton64((bodies[i].position.x - bounds.xmin) * distance, (bodies[i].position.y - bounds.ymin) * distance, (bodies[i].position.z - bounds.zmin) * distance);
        sorts[i].bod = bodies[i];
    }
    return (sorts);
}

int binary_border_search(uint64_t *mortons, int startind, int maxind, unsigned int code, int depth)
{
    // printf("starting binary search, startind is %d, maxind is %d, code is %d, depth is %d\n", startind, maxind, code, depth);
    if (startind == maxind)
        return 0;//empty cell at end of parent
    uint64_t m = mortons[startind];
    m = m << (1 + 3 * depth);
    m = m >> 61;
    if (m != code)
        return 0; // empty cell.
    int step = (maxind - startind) / 2;
    int i = startind;
    while (i < maxind - 1)
    {
        m = mortons[i];
        m = m << (1 + 3 * depth);
        m = m >> 61;
        uint64_t mnext = mortons[i + 1];
        mnext = mnext << (1 + 3 * depth);
        mnext = mnext >> 61;
        if (m == code && mnext != code)
            return (i - startind + 1); //found border
        else if (m == code)
            i += step;//step forward
        else
            i -= step;//step backward
        step /= 2;
        if (step == 0)
            step = 1;
    }
    return (maxind - startind);
}

void split(t_tree *node)
{
	//split this cell into 8 octants
    node->children = (t_tree **)calloc(8, sizeof(t_tree *));
    int depth = node_depth(node);
    unsigned int offset = 0;
    //printf("this cell has %d bodies\n", node->count);
    for (unsigned int i = 0; i < 8; i++)
    {
        node->children[i] = (t_tree *)calloc(1, sizeof(t_tree));
        node->children[i]->bodies = &(node->bodies[offset]);
        node->children[i]->mortons = &(node->mortons[offset]);
        node->children[i]->count = 0;
        node->children[i]->parent = node;
        node->children[i]->children = NULL;
        node->children[i]->bounds = bounds_from_code(node->bounds, i);
		
		//scan through array for borders between 3-bit substring values for this depth
		//these are the dividing lines between octants
        unsigned int j = binary_border_search(node->mortons, offset, node->count, i, depth);
        offset += j;
        node->children[i]->count = j;
        //printf("child %d has %d bodies\n", i, j);
    }
}

void split_tree(t_tree *root)
{
	//we divide the tree until each leaf cell has < leaf threshold bodies.
	//we also have to halt at depth 21, but it is unlikely to need to divide that far.
    if (root->count < LEAF_THRESHOLD || node_depth(root) == 21)
    {
        root->as_single = make_as_single(root);
        //printf("leaf at depth %d with count %d\n", node_depth(root), root->count);
        if (root->count >= LEAF_THRESHOLD)
        {
            for (int i = 0; i < root->count; i++)
                print_cl4(root->bodies[i].position);
        }
        return;
    }
    split(root);
    for (int i = 0; i < 8; i++)
        split_tree(root->children[i]);
    root->as_single = make_as_single(root);
}

t_tree *make_tree(t_body *bodies, int count)
{
	//determine bounding cube of particle set
    t_bounds root_bounds = bounds_from_bodies(bodies, count);
	
	//generate and cache morton codes for each body
    t_sortbod *sorts = make_sortbods(bodies, root_bounds, count);
	
	//sort the bodies by their morton codes
	//they are now arranged on a z-order curve.
    mort_tim_sort(sorts, count);
	
	//copy data back from cached sort structure
    uint64_t *mortons = calloc(count, sizeof(uint64_t));
    for (int i = 0; i < count; i++)
    {
        bodies[i] = sorts[i].bod;
        mortons[i] = sorts[i].morton;;
    }
    t_tree *root = new_tnode(bodies, count, NULL);
    root->bounds = root_bounds;
    root->mortons = mortons;
	
    //recursively divide the tree
	split_tree(root);
    free(sorts);
    free(mortons);
    return (root);
}

static void free_tree(t_tree *t)
{
    if (!t)
        return;
    if (t->children)
        for (int i = 0; i < 8; i++)
            free_tree(t->children[i]);
    free(t->neighbors);
    free(t->children);
    if (t->as_single)
    {
        free(t->as_single->bodies);
        free(t->as_single);
    }
    free(t);
}

void    divide_dataset(t_dispatcher *dispatcher)
{
    clock_t start = clock();
    static t_tree *t;

    if (t != NULL)
        free_tree(t);
    if (DEBUG && DIVIDE_DATASET_DEBUG)
        printf("starting divide_dataset\n");
    printf("there are %ld stars\n", dispatcher->dataset->particle_cnt);
    t = make_tree(dispatcher->dataset->particles, dispatcher->dataset->particle_cnt);
    t_tree **leaves = enumerate_leaves(t);
    printf("leaves enumerated there were %d, assembling neighborhoods\n", count_tree_array(leaves));
    int largest_local = 0;
    int max_depth = 0;
    for (int i = 0; leaves[i]; i++)
    {
        leaves[i]->neighbors = assemble_neighborhood(leaves[i], t);
        if (leaves[i]->count > largest_local)
            largest_local = leaves[i]->count;
        if (node_depth(leaves[i]) > max_depth)
            max_depth = node_depth(leaves[i]);
    }
    printf("the largest cell had %d locals\n", largest_local);
    printf("the deepest cell was at level %d\n", max_depth);
    int lcount = count_tree_array(leaves);
    int wcount = dispatcher->worker_cnt;
    int leaves_per_bundle = (int)ceil((float)lcount / (float)wcount);
	static int bundle_id = 0;
    dispatcher->cells = leaves;
    dispatcher->total_workunits = count_tree_array(leaves);
    dispatcher->cell_count = count_tree_array(leaves);
    //printf("bundling started\n");
    for (int i = 0; i * leaves_per_bundle < lcount; i++)
    {
        t_bundle *b = bundle_leaves(leaves, i * leaves_per_bundle, leaves_per_bundle);
	    printf("bundle would have been %.0fMB\n", (float)b->size / 1048576.0f);
		b->id = bundle_id++;
        queue_enqueue(&dispatcher->bundles, queue_create_new(b));
		sem_post(dispatcher->start_sending);
    }
    //printf("bundling finished\n");
    printf("divide_dataset took %lu cycles total\n", clock() - start);
}
