/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_worker_done_msg.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 16:35:38 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/14 00:24:22 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void 	delete_workunit(t_workunit **workunit)
{
	if (workunit && *workunit)
	{
		free((*workunit)->local_bodies);
		(*workunit)->local_bodies = NULL;
		free((*workunit)->neighborhood);
		(*workunit)->neighborhood = NULL;
		free(*workunit);
		*workunit = NULL;
	}
}

void 	delete_WU(t_WU WU)
{
	free(WU.local_bodies);
	WU.local_bodies = NULL;
	free(WU.neighborhood);
	WU.neighborhood = NULL;
}

t_body	*translate_to_new_dataset(t_dispatcher *d, t_body *old)
{
	t_body *ref;

	ref = (t_body *)(old - d->dataset->particles + d->new_dataset->particles);
	return ref;
}

void integrate_WU_results(t_dispatcher *disp, t_tree *old_cell, t_WU *new_WU)
{
	t_body *start_addr = (t_body *)(old_cell->bodies - disp->dataset->particles + disp->new_dataset->particles);
	memcpy(start_addr, new_WU->local_bodies, old_cell->count * sizeof(t_body));
	async_save(disp, old_cell->bodies - disp->dataset->particles, new_WU);
}

void free_bundle(t_bundle *b)
{
	if (!b)
		return;
	if (DEBUG)
		printf("deleting bundle %d\n", b->id);
    free(b->keys);
    free(b->cells);
    free(b->matches_counts);
    for (int i = 0; i < b->cellcount; i++)
        free(b->matches[i]);
    free(b->matches);
    free(b);
}

void	handle_worker_done_msg(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_WU		complete_WU;
	t_tree		*local_cell;
	t_bundle	*to_delete;

	complete_WU = deserialize_WU(msg);
	free(msg.data);
	local_cell = dispatcher->cells[complete_WU.id];
	integrate_WU_results(dispatcher, local_cell, &complete_WU);
	if (DEBUG && complete_WU.is_last)
		printf("is last workunit of a bundle: %d\n", complete_WU.is_last);
	if (complete_WU.is_last)
	{
		to_delete = queue_pop(&worker->workunit_queue);
		if (DEBUG)
			printf("bundle %d complete\n", to_delete->id);
		free_bundle(to_delete);
	}
	delete_WU(complete_WU);
	pthread_mutex_lock(&dispatcher->workunits_done_mutex);
	dispatcher->workunits_done++;
	if (dispatcher->workunits_done == dispatcher->total_workunits)
		all_workunits_done(dispatcher);
	pthread_mutex_unlock(&dispatcher->workunits_done_mutex);
}
