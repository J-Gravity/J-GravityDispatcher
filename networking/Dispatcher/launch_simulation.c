/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/27 15:55:09 by ssmith           ###   ########.fr       */
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
	if (msg.id == WORK_UNIT_REQUEST)
		handle_workunit_req(dispatcher, worker, msg);
	else if (msg.id == WORK_UNIT_DONE)
		handle_worker_done_msg(dispatcher, worker, msg);
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


	signal(SIGPIPE, SIG_IGN);
	params = (t_thread_handler *)input;
	worker = params->worker;
	cur_worker = (t_worker *)worker->data;
	send_worker_msg(cur_worker, new_message(WORK_UNITS_READY, 1, " "));
	while (1)
	{
		pthread_mutex_lock(&params->dispatcher->worker_list_mutex);
		head = params->dispatcher->workers;
		while (head)
			head = head->next;
		pthread_mutex_unlock(&params->dispatcher->worker_list_mutex);
		msg = get_worker_msg(cur_worker);
		if (msg.error == -1)
			printf("get worker message failed with err %d\n", errno);
		if (msg.error == 0 || msg.error == -1)
		{
			if (cur_worker->workunit_link)
			{
				pthread_mutex_lock(&params->dispatcher->workunits_mutex);
				cur_worker->workunit_link->next = params->dispatcher->workunits;
				params->dispatcher->workunits = cur_worker->workunit_link;
				pthread_mutex_unlock(&params->dispatcher->workunits_mutex);
			}
			cur_worker->workunit_link = NULL;
			pthread_mutex_lock(&params->dispatcher->worker_list_mutex);
			remove_link(&params->dispatcher->workers, worker->data);
			pthread_mutex_unlock(&params->dispatcher->worker_list_mutex);
			free(worker);
			if (cur_worker->socket.fd)
				close(cur_worker->socket.fd);
			kill_me = *cur_worker->tid;
			free(cur_worker->tid);
			free(cur_worker);
			free(params);
			printf("killing this thread...\n");
			return (0);
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

	pthread_mutex_lock(&dispatcher->worker_list_mutex);
	head = dispatcher->workers;
	while (head)
	{	
		cur_worker = (t_worker *)head->data;
		if (cur_worker->tid == 0)
		{
			param = new_thread_handler(dispatcher, head);
			cur_worker->tid = calloc(1, sizeof(pthread_t));
			pthread_create(cur_worker->tid, NULL, handle_worker_connection, param);
		}
		head = head->next;
	}
	pthread_mutex_unlock(&dispatcher->worker_list_mutex);
	sleep(999999);
}
