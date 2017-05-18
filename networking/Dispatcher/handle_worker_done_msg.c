/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_worker_done_msg.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 16:35:38 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/17 22:32:20 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_worker_done_msg(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_workunit	new_workunit;
	t_cell		*local_cell;
	int			i;

	new_workunit = deserialize_workunit(msg);
	local_cell = dispatcher->cells[new_workunit.id];
	i = 0;
	while (i < local_cell->body_count)
	{
		memcpy(local_cell->contained_bodies[i], &new_workunit.local_bodies[i],
				sizeof(t_body));
		i++;
	}
	dispatcher->workunits_done++;
	if (dispatcher->workunits_done == dispatcher->workunits_cnt)
	{
		send_worker_msg(worker, new_message(ACKNOWLEDGED, 0, ""));
		all_workunits_done(dispatcher);
	}
	else if (dispatcher->workunits)
		send_worker_msg(worker, new_message(WORK_UNITS_READY, 0, ""));
}
