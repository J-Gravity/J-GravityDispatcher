/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   event_thread.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 17:06:01 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/03 20:12:19 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void	handle_event(t_worker *worker, t_msg msg)
{
		printf("flag-1\n");
	if (msg.id == WORK_UNITS_READY)
	{
		printf("flag0\n");
		//may need to mutex the socket to avoid conflict with sender_thread
		send_msg(worker->socket.fd, (t_msg){WORK_UNIT_REQUEST, 1, strdup(" ")});
		free(msg.data);
	}
	else if (msg.id == WORK_UNIT)
	{
		printf("flag1\n");
		queue_enqueue(&worker->todo_work, queue_create_new(deserialize_workunit(msg)));
		if (DEBUG)
        	printf("work unit added to local queue\n");
		free(msg.data);
	}
}

static void	*event_thread(void *param)
{
	t_msg		msg;
	t_worker	*worker;

	worker = (t_worker *)param;
	while (1)
	{
		msg = receive_msg(worker->socket.fd);
		if (DEBUG && MSG_DEBUG && MSG_DETAILS_DEBUG)
		{
			printf("done receiving message\n");
			printf("msg status: %d\n", msg.error);
			printf("MSG RECIEVED: [id]=%d", msg.id);
			printf(" size '%d'\n", msg.size);
			printf(" body '%s'\n", msg.data);
		}
		if (msg.error == -1)
			printf("get worker message failed with err %d\n", errno);
		if (msg.error == 0 || msg.error == -1)
		{
		printf("flag-2\n");
			if (DEBUG && NETWORK_DEBUG)
				printf("dispatcher connection terminated! %d\n", worker->socket.fd);
			worker->active = 0;
		}
		else
		{
			handle_event(worker, msg);
		}
		printf("event reached end loop\n");
	}
	return (0);
}

void		launch_event_thread(t_worker *worker)
{
	worker->event_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->event_thread, NULL, event_thread, worker);
}
