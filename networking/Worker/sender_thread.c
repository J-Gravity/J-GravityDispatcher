/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sender_thread.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 19:20:34 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/03 20:14:34 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void *sender_thread(void *param)
{
	t_worker	*worker;
	t_workunit	*workunit;
	t_msg		msg;

	worker = (t_worker *)param;
	while (1)
	{
		if (worker->completed_work && worker->completed_work->count)
		{
			printf("start completed_work\n");
			if (DEBUG)
				printf("sending work unit\n");
			workunit = queue_pop(&worker->completed_work);
			//send complete work unit
			msg = serialize_workunit(*workunit);
			send_msg(worker->socket.fd, msg);
			if (workunit->local_bodies)
				free(workunit->local_bodies);
			free(workunit);
			printf("sent\n");
		}
	}
	return (0);
}

void launch_sender_thread(t_worker *worker)
{
	worker->sender_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->sender_thread, NULL, sender_thread, worker);
}
