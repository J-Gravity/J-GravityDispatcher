/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   divide_dataset.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:43:16 by scollet           #+#    #+#             */
/*   Updated: 2017/05/29 21:26:10 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

#include <math.h>
#define xmid c->bounds.xmax - (c->bounds.xmax - c->bounds.xmin) / 2
#define ymid c->bounds.ymax - (c->bounds.ymax - c->bounds.ymin) / 2
#define zmid c->bounds.zmax - (c->bounds.zmax - c->bounds.zmin) / 2
#define SOFTENING 10000
#define THETA 1.5
#define LEAF_THRESHOLD pow(2, 13)

void print_cl4(cl_float4 v)
{
    printf("x: %f y: %f z: %f w:%f\n", v.x, v.y, v.z, v.w);
}

static t_bounds bounds_from_bodies(t_body **bodies)
{
    //at the start of making the tree, we need a box that bounds all the bodies.
    float xmin = 0, xmax = 0;
    float ymin = 0, ymax = 0;
    float zmin = 0, zmax = 0;

    for (int i = 0; bodies[i]; i++)
    {
        if (bodies[i]->position.x < xmin)
            xmin = bodies[i]->position.x;
        if (bodies[i]->position.x > xmax)
            xmax = bodies[i]->position.x;

        if (bodies[i]->position.y < ymin)
            ymin = bodies[i]->position.y;
        if (bodies[i]->position.y > ymax)
            ymax = bodies[i]->position.y;

        if (bodies[i]->position.z < zmin)
            zmin = bodies[i]->position.z;
        if (bodies[i]->position.z > zmax)
            zmax = bodies[i]->position.z;
    }
    return ((t_bounds){xmin, xmax, ymin, ymax, zmin, zmax});
}

static cl_float4 vadd(cl_float4 a, cl_float4 b)
{
    //add two vectors.
    return ((cl_float4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w});
}

static int count_bodies(t_body **bodies)
{
    //returns the number of bodies in a null terminated array of bodies.
    int i;
    i = 0;
    while (bodies[i])
        i++;
    return (i);
}

static int count_cell_array(t_cell **cells)
{
    //returns the length of a null terminated array of cell pointers
    int i = 0;
    if (!cells)
        return 0;
    while (cells[i])
        i++;
    return (i);
}

static cl_float4 center_of_mass(t_cell *c, t_body **bodies, int *count)
{
    //as an optimization, center_of_mass also counts the bodies and stores that
    cl_float4 v;

    v = (cl_float4){0,0,0,0};
    int i;
    for (i = 0; bodies[i]; i++)
        v = vadd(v, bodies[i]->position);
    *count = i;
    if (v.w == 0)
      	return (cl_float4){xmid, ymid, zmid, 0};
    return ((cl_float4){v.x / v.w, v.y / v.w, v.z / v.w, v.w});
}

static t_cell *init_cell(t_body **bodies, t_cell *parent, t_bounds bounds)
{
    //allocates and sets up a cell struct.
    t_cell *c;

    c = (t_cell *)calloc(1, sizeof(t_cell));
    c->bodies = bodies;
    c->bodycount = 0;
    c->parent = parent;
    c->children = NULL;
    c->bounds = bounds;
    c->center = center_of_mass(c, bodies, &(c->bodycount));
    c->force_bias = (cl_float4){0, 0, 0, 0};
    return (c);
}

static t_octree *init_tree(t_body **bodies, size_t n, t_bounds bounds)
{
    //allocates and sets up a tree and creates its root cell
    t_octree *t;

    t = (t_octree *)calloc(1, sizeof(t_octree));
    t->bodies = bodies;
    t->n_bodies = n;
    t->bounds = bounds;
    t->root = init_cell(bodies, NULL, bounds);
    return (t);
}

static cl_float4 midpoint_from_bounds(t_bounds b)
{
    return (cl_float4){(b.xmax - b.xmin) / 2, (b.ymax - b.ymin) / 2, (b.zmax - b.zmin) / 2};
}

static float multipole_acceptance_criterion(t_cell *us, t_cell *them)
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
    r.x = them->center.x - us_midpoint.x;
    r.y = them->center.y - us_midpoint.y;
    r.z = them->center.z - us_midpoint.z;
    d = sqrt(r.x * r.x + r.y * r.y + r.z * r.z);

    //in normal Barnes-Hut, the MAC is evaluated for every body vs every cell. 
    //we're evaluating it cell to cell for speed and to build good workunits.
    //this could result in some comparisons that should be near being done as far.
    //subtracting half of our cell's width from the measured distance compensates for this adequately.
    //basically, we are measuring as if all the bodies in our cell are right up against the nearest side of our cell.
    d -= (us->bounds.xmax - us->bounds.xmin) / 2;
    return (s/d);
}

static t_cell *single_body_cell(t_cell *cell)
{
    //returns a new cell that's just one body. basically just a wrapper for the body,
    //it's a little inelegant to do this rather than just make a body, but it fits in the existing code well.
    //when this is rewritten for GPU i'll do it differently.
    t_cell *single = (t_cell *)calloc(1, sizeof(t_cell));
    single->bodies = (t_body **)calloc(2, sizeof(t_body *));
    single->bodies[0] = (t_body *)calloc(1, sizeof(t_body));
    single->bodies[0]->position = cell->center;
    single->bodies[0]->velocity = (cl_float4){0, 0, 0, -1}; //this is a signal so we can free it appropriately
    single->bodies[1] = NULL;
    single->bodycount = 1;
    single->bounds = (t_bounds){0,0,0,0,0,0}; // this is also an anti-leak signal
    return (single);
}

static t_cell **find_inners_do_outers(t_cell *cell, t_cell *root, t_octree *t)
{
    t_cell **ret;
    t_cell ***returned;

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

    if (root != t->root && multipole_acceptance_criterion(cell, root) < THETA)
    {
        ret = (t_cell **)calloc(2, sizeof(t_cell *));
        ret[0] = single_body_cell(root);
        ret[1] = NULL;
        return (ret);
    }
    else if (!(root->children))
    {
        ret = (t_cell **)calloc(2, sizeof(t_cell *));
        ret[0] = root;
        ret[1] = NULL;
        return (ret);
    }
    else
    {
        returned = (t_cell ***)calloc(8, sizeof(t_cell **));
        int total = 0;
        for (int i = 0; i < 8; i++)
        {
            returned[i] = find_inners_do_outers(cell, root->children[i], t);
            total += count_cell_array(returned[i]);
        }
        ret = (t_cell **)calloc(total + 1, sizeof(t_cell *));
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

static int boundequ(t_bounds a, t_bounds b)
{
    //simple helper function to determine if two t_bounds are equal.
    if (a.xmin == b.xmin && a.xmax == b.xmax)
        if (a.ymin == b.ymin && a.ymax == b.ymax)
            if (a.zmin == b.zmin && a.zmax == b.zmax)
                return (1);
    return (0);
}

static t_body **bodies_from_cells(t_cell **cells, int *neighborcount)
{
    //given a null terminated list of cells, make a null terminated array of all the bodies in those cells.
    //also sets neighborcount to the length of the array.
    int count;
    t_body **bodies;

    //printf("entering b_f_c\n");
    count = 0;
    for (int i = 0; cells[i]; i++)
        count += cells[i]->bodycount;
    bodies = (t_body **)calloc(count + 1, sizeof(t_body *));
    bodies[count] = NULL;
    int k = 0;
    for (int i = 0; cells[i]; i++)
    {
        memcpy(&(bodies[k]), cells[i]->bodies, cells[i]->bodycount * sizeof(t_body *));
        k += cells[i]->bodycount;
        if (cells[i]->bodycount == 1 && boundequ(cells[i]->bounds, (t_bounds){0, 0, 0, 0, 0, 0}))
        {
            free(cells[i]->bodies);
            free(cells[i]);
        }
    }
    //printf("leaving b_f_c\n");
    *neighborcount = count;
    return (bodies);
}

static t_workunit *new_workunit(t_cell *c, t_body **neighborhood, int neighborcount, int index)
{
    //constructor for workunit.
    t_workunit *w;

    w = (t_workunit *)calloc(1, sizeof(t_workunit));
    w->id = index;
    w->localcount = c->bodycount;
    w->neighborcount = neighborcount;
    w->force_bias = c->force_bias;
    w->local_bodies = c->bodies;
    w->neighborhood = neighborhood;
    return (w);

}

static t_workunit *make_workunit_for_cell(t_cell *cell, t_octree *t, int index)
{
    t_cell **inners;
    t_body **direct_bodies;
    t_workunit *w;
    int neighborcount;

    //skip empty cells
    if (cell->bodycount == 0)
        return NULL;
    //traverse tree and assemble its neighborhood (here "inners," I need to do a sweep to unify naming)
    inners = find_inners_do_outers(cell, t->root, t);
    //use the result to make a list of particles we need to direct compare against
    direct_bodies = bodies_from_cells(inners, &neighborcount);
    free(inners);
    w = new_workunit(cell, direct_bodies, neighborcount, index);
    return (w);
}

static void    paint_bodies_octants(t_body **bodies, t_cell *c)
{
    //mark each body with a value for which octant it will be in after cell is split
    for (int i = 0; i < c->bodycount; i++)
    {
        if (bodies[i]->position.x < xmid)
        {
            if (bodies[i]->position.y < ymid)
            {
                if (bodies[i]->position.z < zmid)
                    bodies[i]->velocity.w = 0;
                else
                    bodies[i]->velocity.w = 1;
            }
            else
            {
                if (bodies[i]->position.z < zmid)
                    bodies[i]->velocity.w = 2;
                else
                    bodies[i]->velocity.w = 3;
            }
        }
        else
        {
            if (bodies[i]->position.y < ymid)
            {
                if (bodies[i]->position.z < zmid)
                    bodies[i]->velocity.w = 6;
                else
                    bodies[i]->velocity.w = 7;
            }
            else
            {
                if (bodies[i]->position.z < zmid)
                    bodies[i]->velocity.w = 4;
                else
                    bodies[i]->velocity.w = 5;
            }
        }
    }
}

static t_body ***scoop_octants(t_body **bodies, int count)
{
    //return 8 arrays of bodies, one for each octant (used after paint_octants)
    int counts[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < count; i++)
        counts[(int)bodies[i]->velocity.w] += 1;
    t_body ***ret = (t_body ***)calloc(8, sizeof(t_body **));
    for (int i = 0; i < 8; i++)
    {
        ret[i] = (t_body **)calloc(counts[i] + 1, sizeof(t_body *));
        ret[i][counts[i]] = NULL;
    }
    int indices[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < count; i++)
    {
        ret[(int)bodies[i]->velocity.w][indices[(int)bodies[i]->velocity.w]] = bodies[i];
        indices[(int)bodies[i]->velocity.w] += 1;
    }
    return (ret);
}

static void divide_cell(t_cell *c)
{
    //split a cell into 8 octants.

    t_bounds subbounds[8];
    //min->max is left->right, bottom->top, near->far
    subbounds[0] = (t_bounds){c->bounds.xmin, xmid, \
                                c->bounds.ymin, ymid, \
                                c->bounds.zmin, zmid}; //bottom left near

    subbounds[6] = (t_bounds){xmid, c->bounds.xmax, \
                                c->bounds.ymin, ymid, \
                                c->bounds.zmin, zmid}; //bottom right near
    subbounds[2] = (t_bounds){c->bounds.xmin, xmid, \
                                ymid, c->bounds.ymax, \
                                c->bounds.zmin, zmid}; // top left near
    subbounds[1] = (t_bounds){c->bounds.xmin, xmid, \
                                c->bounds.ymin, ymid, \
                                zmid, c->bounds.zmax}; // bottom left far

    subbounds[3] = (t_bounds){c->bounds.xmin, xmid, \
                                ymid, c->bounds.ymax, \
                                zmid, c->bounds.zmax}; //top left far
    subbounds[7] = (t_bounds){xmid, c->bounds.xmax, \
                                c->bounds.ymin, ymid, \
                                zmid, c->bounds.zmax}; //bottom right far
    subbounds[4] = (t_bounds){xmid, c->bounds.xmax, \
                                ymid, c->bounds.ymax, \
                                c->bounds.zmin, zmid}; //right top near

    subbounds[5] = (t_bounds){xmid, c->bounds.xmax, \
                                ymid, c->bounds.ymax, \
                                zmid, c->bounds.zmax}; //right top far

    //////Numbering is a bit weird here ^ but the idea is that even indices are near, odd far
                                //0..3 are left, 4..7 are right
                                //0, 1, 6, 7 bottom, 2345 top
    //note: I made up this numbering scheme but it's actually not used anywhere ever.
    t_cell **children = (t_cell **)calloc(8, sizeof(t_cell *));
    paint_bodies_octants(c->bodies, c);
    t_body ***kids = scoop_octants(c->bodies, c->bodycount);
    for (int i = 0; i < 8; i++)
        children[i] = init_cell(kids[i], c, subbounds[i]);
    free(kids);
    c->children = children;
}

static void tree_it_up(t_cell *root)
{
    //recursively flesh out the barnes-hut tree from the root node.
    //just keep splitting into 8 sub-octants and recursing on them until the number of stars in the cell is manageable.
    if (!root)
        return ;
    if (root->bodycount < LEAF_THRESHOLD)
        return ;
    divide_cell(root);
    for (int i = 0; i < 8; i++)
        tree_it_up(root->children[i]);
}

static t_cell **enumerate_leaves(t_cell *root)
{
    //return a linear t_cell** that's all the leaf nodes (ie childless nodes) in the tree

    t_cell **ret;

    if (!root->children)
    {
        ret = (t_cell **)calloc(2, sizeof(t_cell *));
        ret[0] = root;
        ret[1] = NULL;
        return (ret);
    }
    t_cell ***returned = (t_cell ***)calloc(8, sizeof(t_cell **));
    int total = 0;
    for (int i = 0; i < 8; i++)
    {
        returned[i] = enumerate_leaves(root->children[i]);
        total += count_cell_array(returned[i]);
    }
    ret = (t_cell **)calloc(total + 1, sizeof(t_cell *));
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

static t_lst *new_node(t_workunit *w)
{
    t_lst *n;

    n = (t_lst *)calloc(1,sizeof(t_lst));
    n->data_size = sizeof(t_workunit *);
    n->data = w;
    n->next = NULL;
    return (n);
}

int lstlen(t_lst *lst)
{
    if (!lst)
        return (0);
    int i = 0;
    while (lst)
    {
        lst = lst->next;
        i++;
    }
    return (i);
}

static t_lst   *create_workunits(t_octree *t, t_cell **leaves)
{
    t_lst *head = NULL;
    t_lst *tail = NULL;
    t_workunit *w = NULL;
    long sizetotal = 0;

    for (int i = 0; leaves[i]; i++)
    {
        w = make_workunit_for_cell(leaves[i], t, i);
        if (w)
        {
            sizetotal += w->localcount + w->neighborcount;
            if (!head)
            {
                head = new_node(w);
                tail = head;
            }
            else
            {
                tail->next = new_node(w);
                tail = tail->next;
            }
        }
    }
    //printf("%d workunits were %ld stars total\n", lstlen(head), sizetotal);
    return (head);
}

static void recursive_tree_free(t_cell *c)
{
    //the bodies** array in non-empty leaf cells is freed elsewhere (where workunits are freed). 
    //still needs to be freed in inner cells and empty leaves. (technically the bodies** in an empty leaf is size 0 but its still good practice)
    if (!c->children)
    {
        if (c->bodycount == 0)
            free(c->bodies);
        return;
    }
    for (int i = 0; i < 8; i++)
    {
        recursive_tree_free(c->children[i]);
        free(c->children[i]);
    }
    free(c->children);
    free(c->bodies);
}

static void free_tree(t_octree *t)
{
    printf("freeing the tree\n");
    recursive_tree_free(t->root);
    free(t->root);
    free(t);
}

static int unit_size(t_workunit *w)
{
    int total = 12; //id, localcount, neighborcount
    total += w->localcount * sizeof(t_body);
    total += w->neighborcount * sizeof(cl_float4);
    return total;
}

static void tally_workunits(t_lst *units)
{
    int total = 0;
    int local = 0;
    while (units)
    {
        int this = unit_size((t_workunit *)units->data);
        total += this;
        local += ((t_workunit *)units->data)->localcount;
        //printf("WU %d was %dKB\n",((t_workunit *)units->data)->id, this / 1024);
        units = units->next;
    }
	G_workunit_size = total / (1024 * 1024);
    printf("total size of all workunits: %dMB\n", total / (1024 * 1024));
    //printf("total localcount is %d\n", local);
}

void	divide_dataset(t_dispatcher *dispatcher)
{
    static t_octree *t;

    if (t != NULL)
        free_tree(t);
    printf("starting divide_dataset\n");
    t_body **bodies = (t_body **)calloc(dispatcher->dataset->particle_cnt + 1, sizeof(t_body*));
	bodies[dispatcher->dataset->particle_cnt] = NULL;
    for (int i = 0; i < dispatcher->dataset->particle_cnt; i++)
        bodies[i] = &(dispatcher->dataset->particles[i]);
    bodies[dispatcher->dataset->particle_cnt] = NULL;
    t = init_tree(bodies, dispatcher->dataset->particle_cnt, bounds_from_bodies(bodies));
    //printf("tree init done\n");
    tree_it_up(t->root);
    t_cell **leaves = enumerate_leaves(t->root);
    printf("tree is made\n");
    dispatcher->workunits = create_workunits(t, leaves);
    tally_workunits(dispatcher->workunits);
    int len = lstlen(dispatcher->workunits);
    //printf("2^%d stars, max 2^%d per leaf, resulted in %d units\n", (int)log2(dispatcher->dataset->particle_cnt), (int)log2(LEAF_THRESHOLD), len);
    dispatcher->workunits_cnt = len;
    dispatcher->workunits_done = 0;
    dispatcher->cells = leaves;
    dispatcher->cell_count = len;
    printf("workunits made, done divide_dataset\n");
	return ;
}
