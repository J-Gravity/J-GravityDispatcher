/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_worker_done_msg.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 16:35:38 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/17 21:35:43 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_worker_done_msg(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_workunit	new_workunit;
	t_workunit	*old_workunit;
	int			i;

	new_workunit = deserialize_workunit(msg);
	old_workunit = worker->workunit;
	i = 0;
	while (i < old_workunit->cell.body_count)
	{
		memcpy(old_workunit->cell.contained_bodies[i], new_workunit.cell.contained_bodies[i], sizeof(t_body));
		i++;
	}
	old_workunit->complete = 1;
	dispatcher->workunits_done++;
	if (dispatcher->workunits_done == dispatcher->workunits_cnt)
	{
		all_work_units_done(dispatcher);
	}
	send_worker_msg(worker, new_message(ACKNOWLEDGED, 0, ""));
}
