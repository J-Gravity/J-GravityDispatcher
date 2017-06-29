/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   worker_event_thread.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/29 00:50:21 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/29 01:35:28 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include "pthread.h"

static void	handle_worker_msg(t_dispatcher *dispatcher, t_worker *worker,
			t_msg msg)
{
	if (DEBUG && MSG_DEBUG && MSG_DETAILS_DEBUG)
		printf("handling request!\n");
	if (msg.id == WORK_UNIT_REQUEST)	
		handle_workunit_req(dispatcher, worker, msg);
	else if (msg.id == WORK_UNIT_DONE)
	{
		if (METRICS && WORKER_TIME_METRIC)
			G_worker_calcs += time(NULL) - worker->w_calc_time;
		handle_worker_done_msg(dispatcher, worker, msg);
		worker->w_calc_time = 0;
	}
	else
		printf("invalid msg id: %d\n", msg.id);
}

static void	*worker_event_thread(void *input)
{
	t_thread_handler	*params;
	t_lst				*worker_link;
	t_worker			*worker;
	t_msg				msg;

	signal(SIGPIPE, SIG_IGN);
	if (DEBUG && WORKER_DEBUG)
		printf("Launched worker network handler thread!\n");
	params = (t_thread_handler *)input;
	worker_link = params->worker;
	worker = (t_worker *)worker_link->data;
	worker->active = 1;
	while (worker->active)
	{
		msg = receive_msg(worker->socket.fd);
		if (DEBUG && MSG_DEBUG && MSG_DETAILS_DEBUG)
		{
			printf("done receiving message\n");
			printf("msg status: %d\n", msg.error);
			printf("MSG RECIEVED: [id]=%d", msg.id);
			printf(" size '%zu'\n", msg.size);
			printf(" body '%s'\n", msg.data);
   		}
		if (msg.error == -1)
		{
			printf("get worker message failed with err %d\n", errno);
		}
		if (msg.error == 0 || msg.error == -1)
		{
			if (DEBUG && WORKER_DEBUG)
				printf("worker connection terminated! %d\n", worker->socket.fd);
			worker->active = 0;
		}
		else
			handle_worker_msg(params->dispatcher, worker, msg);
	}
	cleanup_worker(params->dispatcher, worker_link);
	free(params);
	if (DEBUG && WORKER_DEBUG)
		printf("killing this thread...\n");
	return (0);
}

void		make_new_event_thread(t_dispatcher *disp, t_lst *worker_link)
{ 
	t_thread_handler	*params;
	t_worker			*worker;

	params = new_thread_handler(disp, worker_link);
	worker = (t_worker *)worker_link->data;
	worker->tid = calloc(1, sizeof(pthread_t));
	pthread_create(worker->tid, NULL,
					worker_event_thread, params);
}