#include "dispatcher.h"
#include <math.h>
#include <limits.h>

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

//#define THREAD_COUNT 4

// t_body *sort_bodies(t_body *bodies, int count)
// {
//     //divide big list of bodies into quarters, qsort them threaded, merge
//     t_sortkit *chunks = (t_sortkit *)calloc(THREAD_COUNT, sizeof(t_sortkit));
//     int chunksize = (int)ceil((float)count / THREAD_COUNT);
//     int offset = 0;
//     for (int i = 0; i < THREAD_COUNT; i++)
//     {
//         chunks[i].arr = &(bodies[offset]);
//         chunks[i].count = count - offset > chunksize ? chunksize : count - offset;
//         chunks[i].offset = 0;
//         offset += chunks[i].count;
//     }
//     pthread_t *sorts = calloc(THREAD_COUNT, sizeof(pthread_t));
//     for (int i = 0; i < THREAD_COUNT; i++)
//         pthread_create(&(sorts[i]), NULL, sort_thread, (void *)&(chunks[i]));
//     for (int i = 0; i < THREAD_COUNT; i++)
//         pthread_join(sorts[i], NULL);

//     //array is now sorted in THREADCOUNT subsections, we just need to merge
//     t_body *sorted = calloc(count, sizeof(t_body));
//     for (int i = 0; i < count; i++)
//     {
//         int lowid = 0;
//         unsigned int min = UINT_MAX;
//         for (int j = 0; j < THREAD_COUNT; j++)
//         {
//             if (chunks[j].offset == chunks[j].count)
//                 continue ;
//             if (morton_body(chunks[j].arr[chunks[j].offset]) < min)
//             {
//                 lowid = j;
//                 min = morton_body(chunks[j].arr[chunks[j].offset]);
//             }
//         }
//         sorted[i] = chunks[lowid].arr[chunks[lowid].offset];
//         chunks[lowid].offset = chunks[lowid].offset + 1;
//     }
//     return (sorted);
// }

int cache_comp(const void *a, const void *b)
{
	t_body *abod = (t_body *)a;
	t_body *bbod = (t_body *)b;
	unsigned int am = *(unsigned int *)&(abod->velocity.w);
	unsigned int bm = *(unsigned int *)&(bbod->velocity.w);
	return (am - bm);
}

int cache_comp_RAW(const void *a, const void *b)
{
	return (*(unsigned int *)((float *)a + 7) - *(unsigned int *)((float *)b + 7));
}

//i should make a generic merge_sorted_arrays that can take n arrays and a comparator function pointer
//later

//so now we have a list of bodies, sorted by their morton codes. we can generate an octree from this quite directly,
//I just need to wrap my head around it.

//it starts with elements like 
//000...
//000...
//000...
//and these are all in the same first-level child.
//i can just recursively build it from this, can't I? it might not be the fastest but it's a good start.

typedef struct s_tree
{
	t_body *bodies;
	int 	count;
	struct s_tree **children;
	struct s_tree *parent;
}				t_tree;

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

void split_2(t_tree *node)
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
		unsigned int j = 0;
		while (j + offset < node->count)
		{
			unsigned int m = *(unsigned int *)&(node->bodies[offset + j].velocity.w);
			m = m << (2 + 3 * depth);
			m = m >> (27 - 3 * depth);
			if (m != i)
				break;
			j++;
		}
		offset += j;
		node->children[i]->count = j;
		//printf("made cell %u, it's got %u bodies in it. offset is now %u\n", i, node->children[i]->count, offset);
	}
}

#define LEAF_THRESHOLD pow(2, 12)

void make_tree(t_tree *root)
{
	if (root->count < LEAF_THRESHOLD || node_depth(root) == 9)
		return;
	//printf("splitting at level %d\n", node_depth(root));
	split_2(root);
	for (int i = 0; i < 8; i++)
		make_tree(root->children[i]);
}

//come back to testing this vs qsort
// static int	partition(int *array, int l, int r)
// {
// 	int		i;
// 	int		j;
// 	int		temp;

// 	i = l;
// 	j = r + 1;
// 	while (1)
// 	{
// 		i++;
// 		while (array[i] <= array[l] && i <= r)
// 			i++;
// 		j--;
// 		while (array[j] > array[l] && j >= l)
// 			j--;
// 		if (i >= j)
// 			break ;
// 		temp = array[i];
// 		array[i] = array[j];
// 		array[j] = temp;
// 	}
// 	temp = array[l];
// 	array[l] = array[j];
// 	array[j] = temp;
// 	return (j);
// }

// void		quick_sort(int *array, int l, int r)
// {
// 	int	j;

// 	if (l < r)
// 	{
// 		j = partition(array, l, r);
// 		quick_sort(array, l, j - 1);
// 		quick_sort(array, j + 1, r);
// 	}
// }

// static t_cell **enumerate_leaves(t_cell *root)
// {
//     //return a linear t_cell** that's all the leaf nodes (ie childless nodes) in the tree

//     t_cell **ret;

//     if (!root->children)
//     {
//         ret = (t_cell **)calloc(2, sizeof(t_cell *));
//         ret[0] = root;
//         ret[1] = NULL;
//         return (ret);
//     }
//     t_cell ***returned = (t_cell ***)calloc(8, sizeof(t_cell **));
//     int total = 0;
//     for (int i = 0; i < 8; i++)
//     {
//         returned[i] = enumerate_leaves(root->children[i]);
//         total += count_cell_array(returned[i]);
//     }
//     ret = (t_cell **)calloc(total + 1, sizeof(t_cell *));
//     for (int i = 0; i < total;)
//     {
//         for (int j = 0; j < 8; j++)
//         {
//             for (int k = 0; returned[j][k]; k++, i++)
//             {
//                 ret[i] = returned[j][k];
//             }
//             free(returned[j]);
//         }
//         free(returned);
//     }
//     ret[total] = NULL;
//     return (ret);
// }

// static t_tree **enumerate_leaves2(t_tree *root)
// {
//     //return a linear t_tree** that's all the leaf nodes (ie childless nodes) in the tree

//     t_tree **ret;

//     if (!root->children)
//     {
//         ret = (t_cell **)calloc(2, sizeof(t_cell *));
//         ret[0] = root;
//         ret[1] = NULL;
//         return (ret);
//     }
//     t_cell ***returned = (t_cell ***)calloc(8, sizeof(t_cell **));
//     int total = 0;
//     for (int i = 0; i < 8; i++)
//     {
//         returned[i] = enumerate_leaves(root->children[i]);
//         total += count_cell_array(returned[i]);
//     }
//     ret = (t_cell **)calloc(total + 1, sizeof(t_cell *));
//     for (int i = 0; i < total;)
//     {
//         for (int j = 0; j < 8; j++)
//         {
//             for (int k = 0; returned[j][k]; k++, i++)
//             {
//                 ret[i] = returned[j][k];
//             }
//             free(returned[j]);
//         }
//         free(returned);
//     }
//     ret[total] = NULL;
//     return (ret);
// }


void tree_test(t_body *bodies, int count)
{

	// printf("testing\n");
	// unsigned int x = UINT_MAX;
	// float f = *(float *) &x;
	// printf("%u, %f, %u\n", x, f, (unsigned int)f);
	// unsigned int y = *(unsigned int *)&f;
	// printf("%u\n", y);

	clock_t start, alloc, sort, end;
	start = clock();
	for (int i = 0; i < count; i++)
	{
		unsigned int m = morton_body(bodies[i]);
		bodies[i].velocity.w = *(float *)&m;
	}
	alloc = clock();
	printf("allocing took %lu cycles\n", alloc - start);
	qsort(bodies, count, sizeof(t_body), cache_comp_RAW);
	sort = clock();
	printf("sorting took %lu cycles\n", sort - alloc);
	t_tree *root = new_tnode(bodies, count, NULL);
	make_tree(root);
	end = clock();
	printf("treeing took %lu cycles\n", end - sort);
}