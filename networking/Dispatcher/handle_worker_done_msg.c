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

void	handle_worker_done_msg(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_WU		new_WU;
	t_cell		*local_cell;
	int			i;

	new_WU = deserialize_WU(msg);
	//printf("deserialized returned WU\n");
	//printf("localcount %d, neighborcount %d, id %d\n", new_workunit.localcount, new_workunit.neighborcount, new_workunit.id);
	local_cell = dispatcher->cells[new_WU.id];
	i = 0;
	// printf("first body\n");
	// print_cl4(new_workunit.local_bodies[0].position);
	// print_cl4(new_workunit.local_bodies[0].velocity);
	while (i < new_WU.localcount)
	{
		local_cell->bodies[i][0] = new_WU.local_bodies[i];
		i++;
	}
	free(new_WU.local_bodies);
	free(new_WU.neighborhood);
	free(((t_workunit *)worker->workunit_link->data)->local_bodies);
	free(((t_workunit *)worker->workunit_link->data)->neighborhood);
	free((t_workunit *)worker->workunit_link->data);
	free(worker->workunit_link);
	worker->workunit_link = NULL;
	//printf("copied the bodies\n");
	pthread_mutex_lock(&dispatcher->workunits_done_mutex);
	dispatcher->workunits_done++;
	if (dispatcher->workunits_done == dispatcher->workunits_cnt)
		all_workunits_done(dispatcher);
	else if (dispatcher->workunits)
		send_worker_msg(worker, new_message(WORK_UNITS_READY, 0, ""));
	pthread_mutex_unlock(&dispatcher->workunits_done_mutex);
}
