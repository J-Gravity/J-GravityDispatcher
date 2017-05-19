/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/18 19:58:21 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include "pthread.h"

static void	*handle_worker_connection(void *input);

static void	handle_worker_msg(t_dispatcher *dispatcher, t_worker *worker,
			t_msg msg)
{
	if (msg.id == WORK_UNIT_REQUEST)
		handle_workunit_req(dispatcher, worker, msg);
	else if (msg.id == WORK_UNIT_DONE)
		handle_worker_done_msg(dispatcher, worker, msg);
	else
		printf("invalid msg id: %d\n", msg.id);
}

static void	make_new_event_thread(t_dispatcher *disp, t_lst *worker_link)
{
	t_thread_handler	*params;
	t_worker			*worker;

	params = new_thread_handler(disp, worker_link);
	worker = (t_worker *)worker_link->data;
	pthread_create(worker->tid, NULL,
					handle_worker_connection, params);
}

static void	*handle_worker_connection(void *input)
{
	t_thread_handler	*params;
	t_lst				*worker;
	t_worker			*cur_worker;
	t_msg				msg;

	printf("Launched worker network handler thread!\n");
	params = (t_thread_handler *)input;
	worker = params->worker;
	cur_worker = (t_worker *)worker->data;
	send_worker_msg(cur_worker, new_message(WORK_UNITS_READY, 0, ""));
	while (1)
	{
		if (worker->next && ((t_worker *)worker->next)->tid == 0)
		{
			make_new_event_thread(params->dispatcher, worker->next);
		}
		msg = get_worker_msg(cur_worker);
		printf("msg status: %d\n", msg.error);
		if (msg.error == -1)
			printf("get worker message failed with err %d\n", errno);
		else if (msg.error == 0)
		{
			printf("worker connection terminated!\n");
			cur_worker->socket.fd = 0;
			printf("attempting to reconnect...\n");
			cur_worker->socket.fd = accept(params->dispatcher->sin.fd, (struct sockaddr *)&(params->dispatcher->sin.addr.sin_addr), &(params->dispatcher->sin.addrlen));
			printf("reconnected\n");
		}
		else
			handle_worker_msg(params->dispatcher, cur_worker, msg);
	}
}

void		launch_simulation(t_dispatcher *dispatcher)
{
	t_worker			*cur_worker;
	t_thread_handler	*param;
	int 				timeout;
	
	timeout = 120;
	printf("begin launch_simulation\n");
	while (!dispatcher->workers)
	{
		printf("There are no workers, simulation cannot start!\n");
		printf("Waiting for workers to connect...\n");
		sleep(5);
		if (--timeout == 0)
		{
			printf("Timeout reached, Simulation aborted!");
			return ;
		}
	}
	printf("flag-0\n");
	param = new_thread_handler(dispatcher, dispatcher->workers);
	cur_worker = (t_worker *)dispatcher->workers->data;
	printf("BREAK\n");
	cur_worker->tid = calloc(1, sizeof(pthread_t));
	printf("ALLOCCED\n");
	pthread_create(cur_worker->tid, NULL, handle_worker_connection, param);
	sleep(999999);
	printf("END\n");
}
