#include "dispatcher.h"
#include <math.h>
#include <limits.h>

#define THETA 1.5
#define LEAF_THRESHOLD pow(2, 14)

// Expands a 10-bit integer into 30 bits
// by inserting 2 zeros after each bit.
unsigned int expandBits(unsigned int v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

// Calculates a 30-bit Morton code for the
// given 3D point located within the unit cube [0,1].
unsigned int morton3D(float x, float y, float z)
{
    x = fmin(fmax(x * 1024.0f, 0.0f), 1023.0f);
    y = fmin(fmax(y * 1024.0f, 0.0f), 1023.0f);
    z = fmin(fmax(z * 1024.0f, 0.0f), 1023.0f);
    unsigned int xx = expandBits((unsigned int)x);
    unsigned int yy = expandBits((unsigned int)y);
    unsigned int zz = expandBits((unsigned int)z);
    return xx * 4 + yy * 2 + zz;
}

unsigned int morton_body(t_body b)
{
    return (morton3D(b.position.x, b.position.y, b.position.z));
}

int cache_comp(const void *a, const void *b)
{
    return (*(unsigned int *)((float *)a + 7) - *(unsigned int *)((float *)b + 7));
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
    //should do this better (ie not use this function) but gets the job done for now
    int depth = 0; //the root has depth 0.
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
        bounds.ymax = parent.xmax;
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

void split(t_tree *node)
{
    node->children = (t_tree **)calloc(8, sizeof(t_tree *));
    int depth = node_depth(node);
    //printf("count of node we're splitting: %d\n", node->count);

    unsigned int offset = 0;
    for (unsigned int i = 0; i < 8; i++)
    {
        node->children[i] = (t_tree *)calloc(1, sizeof(t_tree));
        node->children[i]->bodies = &(node->bodies[offset]);
        node->children[i]->count = 0;
        node->children[i]->parent = node;
        node->children[i]->children = NULL;
        node->children[i]->bounds = bounds_from_code(node->bounds, i);
        unsigned int j = 0;
        while (j + offset < node->count)
        {
            unsigned int m = *(unsigned int *)&(node->bodies[offset + j].velocity.w);
            m = m << (2 + 3 * depth);
            m = m >> 29;
            if (m != i)
                break;
            j++;
        }
        offset += j;
        node->children[i]->count = j;
        //printf("made cell %u, it's got %u bodies in it. offset is now %u\n", i, node->children[i]->count, offset);
    }
}

void split_tree(t_tree *root)
{
    if (root->count < LEAF_THRESHOLD || node_depth(root) == 9)
        return;
    //printf("splitting at level %d\n", node_depth(root));
    split(root);
    for (int i = 0; i < 8; i++)
        split_tree(root->children[i]);
}

static cl_float4 vadd(cl_float4 a, cl_float4 b)
{
    //add two vectors.
    return ((cl_float4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w});
}

static t_tree *make_as_single(t_tree *c)
{
    //as an optimization, center_of_mass also counts the bodies and stores that
    cl_float4 v;

    v = (cl_float4){0,0,0,0};
    if (c->children == NULL)
        for (int i = 0; i < c->count; i++)
            v = vadd(v, c->bodies[i].position);
    else
        for (int i = 0; i < 8; i++)
            if (c->children[i]->count != 0)
                v = vadd(v, c->children[i]->as_single->bodies[0].position);
    t_body *b = calloc(1, sizeof(t_body));
    if (c->count == 0)
        b->position = (cl_float4){0,0,0,0};
    else
        b->position = (cl_float4){v.x / v.w, v.y / v.w, v.z / v.w, v.w};

    t_tree *s = calloc(1, sizeof(t_tree));
    s->parent = NULL;
    s->children = NULL;
    s->count = 1;
    s->as_single = NULL;
    s->bodies = b;
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
        root->as_single = make_as_single(root);
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
    root->as_single = make_as_single(root);
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
        ret[0] = root->as_single; //empty cells are an issue here
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
    float dim = xmax - xmin;
    if (ymax - ymin > dim)
    {
        dim = ymax - ymin;
        min = ymin;
        max = ymax;
    }
    if (zmax - zmin > dim)
    {
        dim = zmax - zmin;
        min = zmin;
        max = zmax;
    }
    return ((t_bounds){min, max, min, max, min, max});
}

static void scale_coords(t_body *bodies, t_bounds bounds, int count)
{
    //all positions need to be translated by -xmin, -ymin, -zmin
    //then scale down by corner-corner bound distance
    //and no reason not to morton while we're here.

    float distance = (bounds.xmax - bounds.xmin) * (bounds.xmax - bounds.xmin);
    distance += (bounds.ymax - bounds.ymin) * (bounds.ymax - bounds.ymin);
    distance += (bounds.zmax - bounds.zmin) * (bounds.zmax - bounds.zmin);
    distance = 1.0 / sqrt(distance);
    for (int i = 0; i < count; i++)
    {
        bodies[i].position = (cl_float4){(bodies[i].position.x - bounds.xmin) * distance, \
                                        (bodies[i].position.y - bounds.ymin) * distance, \
                                        (bodies[i].position.z - bounds.zmin) * distance,
                                        bodies[i].position.w};
        //opportunistic mortenizing
        unsigned int m = morton_body(bodies[i]);
        bodies[i].velocity.w = *(float *)&m;
    }
}

static void descale_coords(t_body *bodies, t_bounds bounds, int count)
{
    //all positions need to be translated by +xmin, +ymin, +zmin
    //then scale up by corner-corner bound distance
    //and no reason not to morton while we're here.

    float distance = (bounds.xmax - bounds.xmin) * (bounds.xmax - bounds.xmin);
    distance += (bounds.ymax - bounds.ymin) * (bounds.ymax - bounds.ymin);
    distance += (bounds.zmax - bounds.zmin) * (bounds.zmax - bounds.zmin);
    distance = sqrt(distance);
    for (int i = 0; i < count; i++)
    {
        bodies[i].position = (cl_float4){bodies[i].position.x * distance + bounds.xmin, \
                                        bodies[i].position.y * distance + bounds.ymin, \
                                        bodies[i].position.z * distance + bounds.zmin,
                                        bodies[i].position.w};
    }
}

t_bundle *bundle_leaves(t_tree **leaves, int offset, int count)
{
    t_dict *dict = create_dict(1000); //fiddle with this number later. it doesnt need to be big.
    
    //take up to Count leaves starting at offset and pour their neighborhoods
    //into the double-key hash. track which leaves we put in.
    t_pair *ids = NULL;
    //int *ids = calloc(count, sizeof(int)); // remember to address offset+count >leavescount
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
        t_tree **adding = leaves[offset + i]->neighbors;
        for (int j = 0; adding[j]; j++)
            dict_insert(dict, adding[j], i);
    }
    return (bundle_dict(dict, ids));
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
    int offset = 0;
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
    return (m);
}

t_tree *make_tree(t_body *bodies, int count)
{
    t_bounds root_bounds = bounds_from_bodies(bodies, count);
    scale_coords(bodies, root_bounds, count);
    qsort(bodies, count, sizeof(t_body), cache_comp);
    t_tree *root = new_tnode(bodies, count, NULL);
    root->bounds = root_bounds;
    split_tree(root);
    descale_coords(bodies, root_bounds, count);
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
    static t_tree *t;

    if (t != NULL)
        free_tree(t);
    if (DEBUG && DIVIDE_DATASET_DEBUG)
        printf("starting divide_dataset\n");
    t = make_tree(dispatcher->dataset->particles, dispatcher->dataset->particle_cnt);
    t_tree **leaves = enumerate_leaves(t);
    printf("leaves enumerated there were %d, assembling neighborhoods\n", count_tree_array(leaves));
    for (int i = 0; leaves[i]; i++)
    {
        //printf("leaf %d has %d locals\n", i, leaves[i]->count);
        leaves[i]->neighbors = assemble_neighborhood(leaves[i], t);
    }
    int wcount = dispatcher->worker_cnt ? dispatcher->worker_cnt : 4;
    int leaves_per_bundle = (count_tree_array(leaves) / wcount) + 1;
    for (int i = 0; i < wcount; i++)
    {
        t_bundle *b = bundle_leaves(leaves, i * leaves_per_bundle, leaves_per_bundle);
        queue_enqueue(&dispatcher->bundles, queue_create_new(b));
    }
    dispatcher->cells = leaves;
    dispatcher->total_workunits = count_tree_array(leaves);
    dispatcher->cell_count = count_tree_array(leaves);
}