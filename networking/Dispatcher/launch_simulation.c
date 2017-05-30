/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/29 21:04:03 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include "pthread.h"
#include <time.h>

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
	{
		handle_worker_done_msg(dispatcher, worker, msg);
		G_worker_calcs = time(NULL) - worker->w_calc_time;
		worker->w_calc_time = 0;
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


	signal(SIGPIPE, SIG_IGN);
	params = (t_thread_handler *)input;
	worker = params->worker;
	cur_worker = (t_worker *)worker->data;
	send_worker_msg(cur_worker, new_message(WORK_UNITS_READY, 1, " "));
	while (1)
	{
		clock_t start = clock(), diff;
		pthread_mutex_lock(&params->dispatcher->worker_list_mutex);
		diff = clock() - start;
		int msec = diff * 1000 / CLOCKS_PER_SEC;
		G_locked = msec%1000;
		head = params->dispatcher->workers;
		while (head)
			head = head->next;
		pthread_mutex_unlock(&params->dispatcher->worker_list_mutex);
//		diff = clock() - start;
//		int msec = diff * 1000 / CLOCKS_PER_SEC;
//printf("handle_worker_connection.head->next lock took %d seconds %d milliseconds\n", msec/1000, msec%1000);
		msg = get_worker_msg(cur_worker);
		if (msg.error == -1)
			printf("get worker message failed with err %d\n", errno);
		if (msg.error == 0 || msg.error == -1)
		{
			printf("worker connection terminated! %d\n", cur_worker->socket.fd);
			if (cur_worker->workunit_link)
			{
				printf("adding lost worker's work unit back to the pool!\n");
				start = clock();
				pthread_mutex_lock(&params->dispatcher->workunits_mutex);
				diff = clock() - start;
				int msec = diff * 1000 / CLOCKS_PER_SEC;
				G_locked = msec%1000;
				cur_worker->workunit_link->next = params->dispatcher->workunits;
				params->dispatcher->workunits = cur_worker->workunit_link;
				pthread_mutex_unlock(&params->dispatcher->workunits_mutex);
//				diff = clock() - start;
//				int msec = diff * 1000 / CLOCKS_PER_SEC;
//printf("handle_worker_connection.remove_cur_worker lock took %d seconds %d milliseconds\n", msec/1000, msec%1000);
			}
			cur_worker->workunit_link = NULL;
			//cleanup thread and worker link
			//detach worker link from the list
			start = clock();
			pthread_mutex_lock(&params->dispatcher->worker_list_mutex);
			diff = clock() - start;
			int msec = diff * 1000 / CLOCKS_PER_SEC;
			G_locked = msec%1000;
			remove_link(&params->dispatcher->workers, worker->data);
			pthread_mutex_unlock(&params->dispatcher->worker_list_mutex);
//			diff = clock() - start;
//			int msec = diff * 1000 / CLOCKS_PER_SEC;
//printf("handle_worker_connection.remove_workunit lock took %d seconds %d milliseconds\n", msec/1000, msec%1000);
			free(worker);
			if (cur_worker->socket.fd)
				close(cur_worker->socket.fd);
			kill_me = *cur_worker->tid;
			free(cur_worker->tid);
			free(cur_worker);
			free(params);
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
	while (!dispatcher->workers)
	{
		printf("Waiting for workers to connect...\n");
		sleep(5);
		if (--timeout == 0)
		{
			printf("Timeout reached, Simulation aborted!");
			return ;
		}
	}
	dispatcher->is_running = 1;

	clock_t start, diff;
	start = clock();
	pthread_mutex_lock(&dispatcher->worker_list_mutex);
	diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	G_locked = msec%1000;
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
