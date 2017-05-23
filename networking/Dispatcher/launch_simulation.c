/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/22 17:43:41 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include "pthread.h"

static void	handle_worker_msg(t_dispatcher *dispatcher, t_worker *worker,
			t_msg msg)
{
	if (msg.id == WORK_UNIT_REQUEST)
	{
		printf("RECIEVED: WORK_UNIT_REQUEST\n");
		handle_workunit_req(dispatcher, worker, msg);
	}
	else if (msg.id == WORK_UNIT_DONE)
	{
		printf("RECIEVED: WORK_UNIT_DONE\n");
		handle_worker_done_msg(dispatcher, worker, msg);
	}
	else
		printf("invalid msg id: %d\n", msg.id);
}

void		*handle_worker_connection(void *input)
{
	t_thread_handler	*params;
	t_lst				*head;
	t_lst				*worker;
	t_worker			*cur_worker;
	t_msg				msg;

	//printf("Launched worker network handler thread!\n");
	params = (t_thread_handler *)input;
	//printf("A!\n");
	worker = params->worker;
	//printf("B!\n");
	cur_worker = (t_worker *)worker->data;
	//printf("C!\n");
	//printf("thread for fd: %d", cur_worker->socket.fd);
	send_worker_msg(cur_worker, new_message(WORK_UNITS_READY, 0, ""));
	//printf("D!\n");
	while (1)
	{
		printf("while 1\n");
		head = params->dispatcher->workers;
		while (head)
		{
			printf("worker: %d\n", ((t_worker*)(head->data))->socket.fd);
			head = head->next;
		}
		// if (worker->next && ((t_worker *)worker->next->data)->tid == 0)
		// {
		// 	printf("new event thread\n");
		// 	make_new_event_thread(params->dispatcher, worker->next);
		// }
		msg = get_worker_msg(cur_worker);
		printf("msg status: %d\n", msg.error);
		printf("MSG RECIEVED: [id]=%d [size]=%d [body]='%s'\n", msg.id, msg.size, msg.data);
		if (msg.error == -1)
		{
			printf("get worker message failed with err %d\n", errno);
			break ;
		}
		else if (msg.error == 0)
		{
			printf("worker connection terminated!\n");
			close(cur_worker->socket.fd);
			cur_worker->socket.fd = 0;
			printf("attempting to reconnect...\n");
			cur_worker->socket.fd = accept(params->dispatcher->sin.fd, (struct sockaddr *)&(params->dispatcher->sin.addr.sin_addr), &(params->dispatcher->sin.addrlen));
			printf("reconnected\n");
		}
		else
			handle_worker_msg(params->dispatcher, cur_worker, msg);
	}
	return (0);
}

void		launch_simulation(t_dispatcher *dispatcher)
{
	t_worker			*cur_worker;
	t_thread_handler	*param;
	int 				timeout;
	t_lst				*head;
	
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
	dispatcher->is_running = 1;
	head = dispatcher->workers;
	while (head)
	{	
		cur_worker = (t_worker *)head->data;
		if (cur_worker->tid == 0)
		{
			//printf("flag-0\n");
			param = new_thread_handler(dispatcher, head);
			//printf("BREAK\n");
			cur_worker->tid = calloc(1, sizeof(pthread_t));
			//printf("ALLOCCED\n");
			pthread_create(cur_worker->tid, NULL, handle_worker_connection, param);
		}
		head = head->next;
	}
	printf("sleeping\n");
	sleep(999999);
	printf("END\n");
}
