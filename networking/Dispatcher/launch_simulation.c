/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/28 18:50:24 by cyildiri         ###   ########.fr       */
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

void		cleanup_worker(t_dispatcher *dispatcher, t_lst *worker_link)
{
	t_worker	*worker;

	worker = (t_worker *)worker_link->data;
	if (worker->workunit_link)
	{
		if (DEBUG)
			printf("adding lost worker's work unit back to the pool!\n");
		pthread_mutex_lock(&dispatcher->workunits_mutex);
		worker->workunit_link->next = dispatcher->workunits;
		dispatcher->workunits = worker->workunit_link;
		pthread_mutex_unlock(&dispatcher->workunits_mutex);
	}
	worker->workunit_link = NULL;
	if (DEBUG)
		printf("removing worker link!\n");
	pthread_mutex_lock(&dispatcher->worker_list_mutex);
	remove_link(&dispatcher->workers, worker);
	pthread_mutex_unlock(&dispatcher->worker_list_mutex);
	free(worker_link);
	if (worker->socket.fd)
	{
		close(worker->socket.fd);
		worker->socket.fd = 0;
	}
	free(worker->tid);
	free(worker);
}

static void	handle_worker_msg(t_dispatcher *dispatcher, t_worker *worker,
			t_msg msg)
{
	if (DEBUG)
		printf("handling request!\n");
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

void 		print_worker_fds(t_dispatcher *dispatcher)
{
	t_lst	*head;

	pthread_mutex_lock(&dispatcher->worker_list_mutex);
	head = dispatcher->workers;
	printf("-------------\n");
	while (head)
	{
		printf("(%p)worker: %d -> (%p)\n", head, ((t_worker*)(head->data))->socket.fd, head->next);
		head = head->next;
	}
	printf("-------------\n");
	pthread_mutex_unlock(&dispatcher->worker_list_mutex);
}

void		*handle_worker_connection(void *input)
{
	t_thread_handler	*params;
	t_lst				*worker_link;
	t_worker			*worker;
	t_msg				msg;

	signal(SIGPIPE, SIG_IGN);
	if (DEBUG)
		printf("Launched worker network handler thread!\n");
	params = (t_thread_handler *)input;
	worker_link = params->worker;
	worker = (t_worker *)worker_link->data;
	send_worker_msg(worker, new_message(WORK_UNITS_READY, 1, " "));
	while (1)
	{
		if (DEBUG)
			print_worker_fds(params->dispatcher);
		msg = get_worker_msg(worker);
		if (DEBUG)
			printf("done receiving message\n");
		if (DEBUG && MSG_DEBUG)
		{
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
			if (DEBUG)
				printf("worker connection terminated! %d\n", worker->socket.fd);
			cleanup_worker(params->dispatcher, worker_link);
			free(params);
			if (DEBUG)
				printf("killing this thread...\n");
			return (0);
		}
		else
			handle_worker_msg(params->dispatcher, worker, msg);
		//free(msg.data);
	}
	return (0);
}

void		launch_worker_event_threads(t_dispatcher *dispatcher)
{
	t_lst				*head;
	t_worker			*cur_worker;
	t_thread_handler	*param;

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
	launch_worker_event_threads(dispatcher);
	printf("sleeping\n");
	sleep(999999);
	printf("END\n");
}
