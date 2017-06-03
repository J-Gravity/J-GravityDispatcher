/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   event_thread.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 17:06:01 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/02 18:28:14 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void handle_event(t_worker *worker, t_msg msg)
{
	if (msg->id == WORK_UNITS_READY)
	{
		send_msg(conn_socket, (t_msg){WORK_UNIT_REQUEST, 1, strdup(" ")});
	}
	else if (msg->id == WORK_UNIT)
	{
		//add work unit to the dispatcher->todo_work queue
		//free msg
	}
}

static void *event_thread(void *param)
{
	t_msg		msg;
	t_worker	*worker;

	worker = (t_worker *)param;
	while (worker->active)
	{
		msg = receive_msg(worker->socket.fd));
		if (DEBUG && MSG_DEBUG && MSG_DETAILS_DEBUG)
		{
			printf("done receiving message\n");
			printf("msg status: %d\n", msg.error);
			printf("MSG RECIEVED: [id]=%d", msg.id);
			printf(" size '%d'\n", msg.size);
			printf(" body '%s'\n", msg.data);
	}
		if (msg.error == -1)
		{
			printf("get worker message failed with err %d\n", errno);
		}
		if (msg.error == 0 || msg.error == -1)
		{
			if (DEBUG && WORKER_DEBUG)
				printf("dispatcher connection terminated! %d\n", worker->socket.fd);
			worker->active = 0;
		}
		else
			handle_event(worker, msg);
	}
	return (0);
}

launch_event_thread(t_worker *worker)
{
	pthread_create(worker->event_thread, NULL, event_thread, worker);
}