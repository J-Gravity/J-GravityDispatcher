/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   divide_dataset.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmclaugh <pmclaugh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:43:16 by scollet           #+#    #+#             */
/*   Updated: 2017/05/18 20:01:33 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

#include <math.h>
#define xmid c->bounds.xmax - (c->bounds.xmax - c->bounds.xmin) / 2
#define ymid c->bounds.ymax - (c->bounds.ymax - c->bounds.ymin) / 2
#define zmid c->bounds.zmax - (c->bounds.zmax - c->bounds.zmin) / 2
#define SOFTENING 100000
#define THETA 1.5
#define LEAF_THRESHOLD pow(2, 13)

static t_bounds bounds_from_bodies(t_body **bodies)
{
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

static cl_float4 center_of_mass(t_cell *c, t_body **bodies)
{
    cl_float4 v;

    v = (cl_float4){0,0,0,0};

    for (int i = 0; bodies[i]; i++)
        v = vadd(v, bodies[i]->position);
    if (v.w == 0)
      	return (cl_float4){xmid, ymid, zmid, 0};
    return ((cl_float4){v.x / v.w, v.y / v.w, v.z / v.w, v.w});
}

static t_cell *init_cell(t_body **bodies, t_cell *parent, t_bounds bounds)
{
    //allocates and sets up a cell struct.
    t_cell *c;

    c = (t_cell *)malloc(sizeof(t_cell));
    c->bodies = bodies;
    c->parent = parent;
    c->children = NULL;
    c->bounds = bounds;
    c->center = center_of_mass(c, bodies);
    c->force_bias = (cl_float4){0, 0, 0, 0};
    return (c);
}

static t_octree *init_tree(t_body **bodies, size_t n, t_bounds bounds)
{
    //allocates and sets up a tree and creates its root cell
    t_octree *t;

    t = (t_octree *)malloc(sizeof(t_octree));
    t->bodies = bodies;
    t->n_bodies = n;
    t->bounds = bounds;
    t->root = init_cell(bodies, NULL, bounds);
    return (t);
}

static void pair_force_cell(t_cell *i, t_cell *j)
{
    //compute the force between two distant cells, treating them as single particles
    cl_float4 r;

    r.x = j->center.x - i->center.x;
    r.y = j->center.y - i->center.y;
    r.z = j->center.z - i->center.z;

    float distSq = r.x * r.x + r.y * r.y + r.z * r.z + SOFTENING;
    float invDist = 1.0 / sqrt(distSq);
    float invDistCube = invDist * invDist * invDist;
    float f = j->center.w * invDistCube * i->center.w > 0 ? 1 : -1;
    i->force_bias = vadd(i->force_bias, (cl_float4){r.x * f, r.y * f, r.z * f});
}

static float multipole_acceptance_criterion(t_cell *us, t_cell *them)
{
    //assess whether a cell is "near" or "far" for the sake of barnes-hut
    //if the value returned is less than THETA, that cell is far
    float s;
    float d;
    cl_float4 r;

    s = them->bounds.xmax - them->bounds.xmin;
    r.x = them->center.x - us->center.x;
    r.y = them->center.y - us->center.y;
    r.z = them->center.z - us->center.z;
    d = sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
    if (d == 0)
        return (0);
    return (s/d);
}

static t_cell **find_inners_do_outers(t_cell *cell, t_cell *root, t_octree *t)
{
    t_cell **ret;
    t_cell ***returned;

    /*
        recursively flow through the tree, determining if cells are near or far from
        the cell we're currently considering. We skip the root.
        
        If the cell is far away (m_a_c < THETA), that cell is far enough away to treat as 1 particle.
        we compute that force and add it to the total force on our cell (force_bias).
        
        if the cell is nearby and childless (ie leaf), it is near enough that direct calculation is necessary,
        so it returns a null-terminated array just containing a pointer to the cell.

        if the cell is nearby and has children, we recurse down to its children.
        we make space for the 8 arrays that will be returned (some might be null)
        then we copy them into one final array and return it.

        in this way, we end up with all necessary distant calculations done
        (and the net resulting force stored in cell->force_bias)
        and we have a nice handy list of all the cells whose bodies we'll need to compare against.
    */

    if (root != t->root && multipole_acceptance_criterion(cell, root) < THETA)
    {
        pair_force_cell(cell, root);
        return (NULL);
    }
    else if (!(root->children))
    {
        ret = (t_cell **)malloc(sizeof(t_cell *) * 2);
        ret[0] = root;
        ret[1] = NULL;
        return (ret);
    }
    else
    {
        returned = (t_cell ***)malloc(sizeof(t_cell **) * 8);
        int total = 0;
        for (int i = 0; i < 8; i++)
        {
            returned[i] = find_inners_do_outers(cell, root->children[i], t);
            total += count_cell_array(returned[i]);
        }
        ret = (t_cell **)malloc(sizeof(t_cell *) * (total + 1));
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

static t_body **bodies_from_cells(t_cell **cells)
{
    //given a null terminated list of cells, make an array of all the bodies in those cells.
    int count;
    t_body **bodies;

    count = 0;
    for (int i = 0; cells[i]; i++)
        count += count_bodies(cells[i]->bodies);
    bodies = (t_body **)malloc(sizeof(t_body *) * (count + 1));
    bodies[count] = NULL;
    int k = 0;
    for (int i = 0; cells[i]; i++)
    {
        for (int j = 0; cells[i]->bodies[j]; j++, k++)
            bodies[k] = cells[i]->bodies[j];
    }
    return (bodies);
}

static t_workunit *new_workunit(t_body **local, t_body **neighborhood, cl_float4 force_bias, int index)
{
    //constructor for workunit.
    //note that memory is copied here.
    t_workunit *w;

    w = (t_workunit *)malloc(sizeof(t_workunit));
    w->id = index;
    w->localcount = count_bodies(local);
    w->neighborcount = count_bodies(neighborhood);
    w->force_bias = force_bias;
    w->local_bodies = (t_body *)malloc(sizeof(t_body) * w->localcount);
    w->neighborhood = (t_body *)malloc(sizeof(t_body) * w->neighborcount);
    for (int i = 0; i < w->localcount; i++)
        w->local_bodies[i] = local[i][0];
    for (int i = 0; i < w->neighborcount; i++)
        w->neighborhood[i] = neighborhood[i][0];
    free(local);
    free(neighborhood);
    return (w);

}

static t_workunit *make_workunit_for_cell(t_cell *cell, t_octree *t, int index)
{
    t_cell **inners;
    t_body **direct_bodies;

    //skip empty cells (yes there are empty cells)
    if (count_bodies(cell->bodies) == 0)
        return NULL;
    //traverse tree doing faraway calculations and enumerating nearby cells (see above)
    inners = find_inners_do_outers(cell, t->root, t);
    //use the result to make a list of particles we need to direct compare against
    direct_bodies = bodies_from_cells(inners);
    return (new_workunit(cell->bodies, direct_bodies, cell->force_bias, index));
}

static void    paint_bodies_octants(t_body **bodies, t_cell *c)
{
    //mark each body with a value for which octant it will be in after cell is split
    for (int i = 0; bodies[i]; i++)
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

static t_body ***scoop_octants(t_body **bodies)
{
    //return 8 arrays of bodies, one for each octant (used after paint_octants)
    t_body ***ret = (t_body ***)malloc(sizeof(t_body **) * 9);
    ret[8] = NULL;
    int count = count_bodies(bodies);
    for (int i = 0; i < 8; i++)
    {
        ret[i] = (t_body **)malloc(sizeof(t_body *) * (count + 1));
        ret[i][count] = NULL;
    }
    int indices[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; bodies[i]; i++)
    {
        ret[(int)bodies[i]->velocity.w][indices[(int)bodies[i]->velocity.w]] = bodies[i];
        indices[(int)bodies[i]->velocity.w] += 1;
    }
    int sum = 0;
    for (int i = 0; i < 8; i++)
    {
        ret[i][indices[i]] = NULL;
        sum += indices[i];
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
    t_cell **children = (t_cell **)malloc(sizeof(t_cell *) * 9);
    paint_bodies_octants(c->bodies, c);
    t_body ***kids = scoop_octants(c->bodies);
    for (int i = 0; i < 8; i++)
        children[i] = init_cell(kids[i], c, subbounds[i]);
    children[8] = NULL;
    c->children = children;
}

static void tree_it_up(t_cell *root)
{
    //recursively flesh out the barnes-hut tree from the root node.
    if (!root)
        return ;
    if (count_bodies(root->bodies) < LEAF_THRESHOLD)
        return ;
    divide_cell(root);
    for (int i = 0; i < 8; i++)
        tree_it_up(root->children[i]);
}

static t_cell **enumerate_leaves(t_cell *root)
{
    //goal is to return a linear t_cell** that's all the leaf nodes in the tree

    t_cell **ret;

    if (!root->children)
    {
        ret = (t_cell **)malloc(sizeof(t_cell *) * 2);
        ret[0] = root;
        ret[1] = NULL;
        return (ret);
    }
    t_cell ***returned = (t_cell ***)malloc(sizeof(t_cell **) * 8);
    int total = 0;
    for (int i = 0; i < 8; i++)
    {
        returned[i] = enumerate_leaves(root->children[i]);
        total += count_cell_array(returned[i]);
    }
    ret = (t_cell **)malloc(sizeof(t_cell *) * (total + 1));
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

    n = (t_lst *)malloc(sizeof(t_lst));
    n->data_size = sizeof(t_workunit);
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

    for (int i = 0; leaves[i]; i++)
    {
        t_workunit *w = make_workunit_for_cell(leaves[i], t, i);
        if (w)
        {
            if (!head)
            {
                head = new_node(w);
                tail = head;
            }
            else
            {
                tail->next = new_node(make_workunit_for_cell(leaves[i], t, i));
                tail = tail->next;
            }
        }
    }
    return (head);
}

static void recursive_tree_free(t_cell *c)
{
    if (!c->children)
        return;
    for (int i = 0; i < 8; i++)
    {
        recursive_tree_free(c->children[i]);
        free(c->bodies);
        free(c->children[i]);
    }
}

static void free_tree(t_octree *t)
{
    recursive_tree_free(t->root);
    free(t->root);
}

void	divide_dataset(t_dispatcher *dispatcher)
{
	printf("start divide\n");
    t_body **bodies = (t_body **)malloc(sizeof(t_body*) * (dispatcher->dataset->particle_cnt + 1));
    printf("f0\n");
	bodies[dispatcher->dataset->particle_cnt] = NULL;
    printf("f1\n");
    for (int i = 0; i < dispatcher->dataset->particle_cnt; i++)
        bodies[i] = &(dispatcher->dataset->particles[i]);
    printf("f2\n");
    t_octree *t = init_tree(bodies, dispatcher->dataset->particle_cnt, bounds_from_bodies(bodies));
    printf("f3\n");
    tree_it_up(t->root);
    printf("f4\n");
    t_cell **leaves = enumerate_leaves(t->root);
    printf("f5\n");
    dispatcher->workunits = create_workunits(t, leaves);
    printf("f6\n");
    int len = lstlen(dispatcher->workunits);
    printf("f7\n");
    dispatcher->workunits_cnt = len;
    printf("f8\n");
    dispatcher->workunits_done = 0;
    printf("f9\n");
    dispatcher->cells = leaves;
    printf("f10\n");
    dispatcher->cell_count = len;
    printf("f11\n");
    free_tree(t); //bodies is freed in here
	printf("fin divide\n");
	return ;
}
