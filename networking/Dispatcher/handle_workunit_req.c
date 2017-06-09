/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_workunit_req.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/08 18:10:35 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_workunit_req(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_workunit *workunit;

	while (worker->active && dispatcher->workunits->count > 0)
	{
		workunit = queue_pop(&dispatcher->workunits);
		queue_enqueue(&worker->workunit_queue, queue_create_new(*workunit));
		if (workunit)
			send_workunit(worker, workunit);
		G_sent_wu++;
	}
	free(msg.data);
}
