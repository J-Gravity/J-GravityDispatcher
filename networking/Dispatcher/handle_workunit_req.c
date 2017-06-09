/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_workunit_req.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/09 00:16:23 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_workunit_req(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_bundle *bundle;

	while (worker->active && dispatcher->bundles->count > 0)
	{
		bundle = queue_pop(&dispatcher->bundles);
		queue_enqueue(&worker->workunit_queue, queue_create_new(*bundle));
		if (bundle)
			send_bundle(worker, bundle, dispatcher->cells);
		G_sent_wu++;
	}
	free(msg.data);
}
