/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_worker_done_msg.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 16:35:38 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/18 15:44:04 by ssmith           ###   ########.fr       */
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
	while (i < old_workunit->localcount)
	{
		memcpy(&old_workunit->neighborhood[i], &new_workunit.local_bodies[i], sizeof(t_body));
		i++;
	}
	dispatcher->workunits_done++;
	if (dispatcher->workunits_done == dispatcher->workunits_cnt)
	{
		all_workunits_done(dispatcher);
	}
	send_worker_msg(worker, new_message(ACKNOWLEDGED, 0, ""));
}
