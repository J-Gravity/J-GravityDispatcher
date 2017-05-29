/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_worker_done_msg.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 16:35:38 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/23 18:11:36 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_body *translate_to_new_dataset(t_dispatcher *d, t_body **old, int index)
{
	t_body *ref;

	ref = old[index];
	ref = (t_body *)(ref - d->dataset->particles + d->new_dataset->particles);
	return ref;
}

void	handle_worker_done_msg(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_WU		new_WU;
	t_cell		*local_cell;
	int			i;

	printf("handling done msg\n");
	new_WU = deserialize_WU(msg);
	local_cell = dispatcher->cells[new_WU.id];
	i = 0;
	while (i < new_WU.localcount)
	{
		t_body *dest;

		dest = translate_to_new_dataset(dispatcher, local_cell->bodies, i);
		*dest = new_WU.local_bodies[i];
		i++;
	}
	free(new_WU.local_bodies);
	free(((t_workunit *)worker->workunit_link->data)->local_bodies);
	free(((t_workunit *)worker->workunit_link->data)->neighborhood);
	free((t_workunit *)worker->workunit_link->data);
	free(worker->workunit_link);
	free(msg.data);
	worker->workunit_link = NULL;
	//printf("copied the bodies\n");
	pthread_mutex_lock(&dispatcher->workunits_done_mutex);
	dispatcher->workunits_done++;
	//printf("done %d of %d workunits\n", dispatcher->workunits_done, dispatcher->workunits_cnt);
	if (dispatcher->workunits_done == dispatcher->workunits_cnt)
		all_workunits_done(dispatcher);
	else if (dispatcher->workunits)
	{
		t_msg m = new_message(WORK_UNITS_READY, 0, "");
		send_worker_msg(worker, m);
		free(m.data);
	}
	pthread_mutex_unlock(&dispatcher->workunits_done_mutex);
}
