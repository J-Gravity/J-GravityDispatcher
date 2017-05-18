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
	t_workunit	*new_workunit;
	t_cell		*local_cell;
	int			i;

	new_workunit = deserialize_workunit(msg);
  local_cell = dispatcher->cells[new_workunit->id];
	i = 0;
	while (i < new_workunit->localcount)
	{
		memcpy(local_cell->bodies[i], &new_workunit->local_bodies[i],
				sizeof(t_body));
		i++;
	}
	dispatcher->work_units_done++;
	if (dispatcher->work_units_done == dispatcher->work_units_cnt)
		all_work_units_done(dispatcher);
	else if (dispatcher->work_units)
		send_worker_msg(worker, new_message(WORK_UNITS_READY, 0, ""));
}
