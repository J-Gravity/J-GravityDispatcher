/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/27 00:18:18 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include "pthread.h"


t_lst 		*remove_link(t_lst **list, void *data)
{
	t_lst	*head;
	t_lst	*last;

	head = *list;
	last = *list;
	if (head && head->data == data)
	{
		*list = head->next;
		return (head);
	}
	while (head)
	{
		if (head->data == data)
		{
			last->next = head->next;
			return (head);
		}
		last = head;
		head = head->next;
	}
	return (NULL);
}

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
	pthread_t			kill_me;

	//printf("Launched worker network handler thread!\n");
	params = (t_thread_handler *)input;
	//printf("A!\n");
	worker = params->worker;
	//printf("B!\n");
	cur_worker = (t_worker *)worker->data;
	//printf("C!\n");
	//printf("thread for fd: %d", cur_worker->socket.fd);
	send_worker_msg(cur_worker, new_message(WORK_UNITS_READY, 1, " "));
	//printf("D!\n");
	while (1)
	{
		//printf("while 1\n");
		pthread_mutex_lock(&params->dispatcher->worker_list_mutex);
				//printf("worker list mutex locked!\n");
		head = params->dispatcher->workers;
		printf("-------------\n");
		while (head)
		{
			printf("worker: %d\n", ((t_worker*)(head->data))->socket.fd);
			head = head->next;
		}
		printf("-------------\n");
		pthread_mutex_unlock(&params->dispatcher->worker_list_mutex);
				//printf("worker list mutex unlocked!\n");
		// if (worker->next && ((t_worker *)worker->next->data)->tid == 0)
		// {
		// 	printf("new event thread\n");
		// 	make_new_event_thread(params->dispatcher, worker->next);
		// }

		//printf("getting worker msg: worker %d\n", cur_worker->socket.fd);
		msg = get_worker_msg(cur_worker);
		//printf("msg status: %d\n", msg.error);
		// printf("MSG RECIEVED: [id]=%d", msg.id);
		// printf(" size '%d'\n", msg.size);
		// printf(" body '%s'\n", msg.data);
		if (msg.error == -1)
		{
			printf("get worker message failed with err %d\n", errno);
		}
		if (msg.error == 0 || msg.error == -1)
		{
			printf("worker connection terminated! %d\n", cur_worker->socket.fd);
			if (cur_worker->workunit_link)
			{
				printf("adding lost worker's work unit back to the pool!\n");
				pthread_mutex_lock(&params->dispatcher->workunits_mutex);
				cur_worker->workunit_link->next = params->dispatcher->workunits;
				params->dispatcher->workunits = cur_worker->workunit_link;
				pthread_mutex_unlock(&params->dispatcher->workunits_mutex);
			}
			cur_worker->workunit_link = NULL;
			//cleanup thread and worker link
			//detach worker link from the list
			pthread_mutex_lock(&params->dispatcher->worker_list_mutex);
			remove_link(&params->dispatcher->workers, worker->data);
			pthread_mutex_unlock(&params->dispatcher->worker_list_mutex);
				//printf("worker list mutex unlocked!\n");
			//printf("free worker link\n");
			free(worker);
			//printf("close the socket fd\n");
			if (cur_worker->socket.fd)
				close(cur_worker->socket.fd);
			kill_me = *cur_worker->tid;
			//printf("free the pthread struct\n");
			free(cur_worker->tid);
			//printf("free the worker struct\n");
			free(cur_worker);
			//printf("free the params\n");
			free(params);
			printf("killing this thread...\n");
			return (0);
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

	pthread_mutex_lock(&dispatcher->worker_list_mutex);
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
	pthread_mutex_unlock(&dispatcher->worker_list_mutex);
	printf("sleeping\n");
	sleep(999999);
	printf("END\n");
}
