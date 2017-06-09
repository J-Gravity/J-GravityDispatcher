/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   event_thread.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 17:06:01 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/03 21:05:26 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void	handle_event(t_worker *worker, t_msg msg)
{
	if (msg.id == WORK_UNITS_READY)
	{
		//may need to mutex the socket to avoid conflict with sender_thread
		send_msg(worker->socket.fd, (t_msg){WORK_UNIT_REQUEST, 1, strdup(" ")});
		free(msg.data);
	}
	else if (msg.id == WORK_UNIT)
	{
		int count;
		t_workunit *WUs = unbundle_workunits(deserialize_bundle(msg), &count);
		for (int i = 0; i < count; i++)
		{
			queue_enqueue(&worker->todo_work, queue_create_new(WUs[i]));
			sem_post(worker->calc_thread_sem);
		}
		if (DEBUG)
        	printf("EVENT- work units from bundle added to local queue\n");
		free(msg.data);
	}
}

static void	*event_thread(void *param)
{
	t_msg		msg;
	t_worker	*worker;

	worker = (t_worker *)param;
	while (worker->active)
	{
		msg = receive_msg(worker->socket.fd);
		if (DEBUG && MSG_DEBUG && MSG_DETAILS_DEBUG)
		{
			printf("EVENT- done receiving message\n");
			printf("EVENT- msg status: %d\n", msg.error);
			printf("EVENT- MSG RECIEVED: [id]=%d", msg.id);
			printf("EVENT- size '%d'\n", msg.size);
			printf("EVENT- body '%s'\n", msg.data);
		}
		if (msg.error == -1)
			printf("EVENT- get worker message failed with err %d\n", errno);
		if (msg.error == 0 || msg.error == -1)
		{
			if (DEBUG && NETWORK_DEBUG)
				printf("EVENT- dispatcher connection terminated! %d\n", worker->socket.fd);
			worker->active = 0;
		}
		else
			handle_event(worker, msg);
		if (DEBUG)
			printf("EVENT- finished event_thread\n");
	}
	printf("EVENT- exiting event thread\n");
	sem_post(worker->exit_sem);
	return (0);
}

void		launch_event_thread(t_worker *worker)
{
	worker->active = 1;
	worker->event_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->event_thread, NULL, event_thread, worker);
}
