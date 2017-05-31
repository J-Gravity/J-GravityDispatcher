/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_worker_done_msg.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 16:35:38 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/30 15:28:41 by cyildiri         ###   ########.fr       */
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

void integrate_WU_results(t_dispatcher *disp, t_cell *old_cell, t_WU *new_WU)
{
	t_body	*old_body;
	int		i;
  
	i = 0;
	while (i < new_WU->localcount)
	{
		old_body = translate_to_new_dataset(disp, old_cell->bodies[i]);
		*old_body = new_WU->local_bodies[i];
		i++;
	}
}

void	handle_worker_done_msg(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_WU		complete_WU;
	t_cell		*local_cell;

	complete_WU = deserialize_WU(msg);
	free(msg.data);
	local_cell = dispatcher->cells[complete_WU.id];
	integrate_WU_results(dispatcher, local_cell, &complete_WU);
	delete_WU(complete_WU);
	delete_workunit((t_workunit **)&worker->workunit_link->data);
	free(worker->workunit_link);
	worker->workunit_link = NULL;
	//printf("copied the bodies\n");
	pthread_mutex_lock(&dispatcher->workunits_done_mutex);
	dispatcher->workunits_done++;
	pthread_mutex_unlock(&dispatcher->workunits_done_mutex);
	//printf("done %d of %d workunits\n", dispatcher->workunits_done, dispatcher->workunits_cnt);
	if (dispatcher->workunits_done == dispatcher->workunits_cnt)
		all_workunits_done(dispatcher);
	else if (dispatcher->workunits)
	{
		t_msg m = new_message(WORK_UNITS_READY, 0, "");
		send_worker_msg(worker, m);
		free(m.data);
	}
	
}
