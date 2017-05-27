/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/26 23:02:56 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include "pthread.h"

static void	handle_worker_msg(t_dispatcher *dispatcher, t_worker *worker,
			t_msg msg)
{
	//printf("handle worker done msg\n");
	if (msg.id == WORK_UNIT_REQUEST)
	{
		handle_workunit_req(dispatcher, worker, msg);
	}
	else if (msg.id == WORK_UNIT_DONE)
	{
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

	params = (t_thread_handler *)input;
	worker = params->worker;
	cur_worker = (t_worker *)worker->data;
	send_worker_msg(cur_worker, new_message(WORK_UNITS_READY, 1, " "));
	while (1)
	{
		head = params->dispatcher->workers;
		while (head)
			head = head->next;
		msg = get_worker_msg(cur_worker);
		if (msg.error == 0 || msg.error == -1)
		{
			if (msg.error == -1)
			{
				printf("msg.error == -1\n");
				sleep(5);
			}
			printf("worker connection terminated!\n");
			close(cur_worker->socket.fd);
			cur_worker->socket.fd = 0;
			if (cur_worker->workunit_link)
			{
				printf("adding lost worker's work unit back to the pool!\n");
				pthread_mutex_lock(&params->dispatcher->workunits_mutex);
				printf("mutex locked!\n");
				cur_worker->workunit_link->next = params->dispatcher->workunits;
				printf("link->next = list head!\n");
				params->dispatcher->workunits = cur_worker->workunit_link;
				printf("list head = link!\n");
				pthread_mutex_unlock(&params->dispatcher->workunits_mutex);
				printf("mutex unlocked!\n");
			}
			cur_worker->workunit_link = NULL;
			printf("attempting to reconnect...\n");
			cur_worker->socket.fd = accept(params->dispatcher->sin.fd, (struct sockaddr *)&(params->dispatcher->sin.addr.sin_addr), &(params->dispatcher->sin.addrlen));
			printf("reconnected\n");
			send_worker_msg(cur_worker, new_message(WORK_UNITS_READY, 1, " "));
		}
		else
			handle_worker_msg(params->dispatcher, cur_worker, msg);
		//free(msg.data);
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
