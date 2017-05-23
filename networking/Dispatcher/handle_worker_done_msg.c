/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_worker_done_msg.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 16:35:38 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/18 16:22:44 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_worker_done_msg(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_workunit	new_workunit;
	t_cell		*local_cell;
	int			i;

	new_workunit = deserialize_workunit2(msg);
	printf("deserialized returned WU\n");
	printf("localcount %d, neighborcount %d, id %d\n", new_workunit.localcount, new_workunit.neighborcount, new_workunit.id);
	local_cell = dispatcher->cells[new_workunit.id];
	i = 0;
	// printf("first body\n");
	// print_cl4(new_workunit.local_bodies[0].position);
	// print_cl4(new_workunit.local_bodies[0].velocity);
	while (i < new_workunit.localcount)
	{
		local_cell->bodies[i][0] = new_workunit.local_bodies[i];
		i++;
	}
	printf("copied the bodies\n");
	dispatcher->workunits_done++;
	if (dispatcher->workunits_done == dispatcher->workunits_cnt)
		all_workunits_done(dispatcher);
	else if (dispatcher->workunits)
		send_worker_msg(worker, new_message(WORK_UNITS_READY, 0, ""));
}
