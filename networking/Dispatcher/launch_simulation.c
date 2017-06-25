/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/15 15:17:17 by cyildiri         ###   ########.fr       */
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
	if (worker->workunit_queue->count > 0)
	{
		if (DEBUG && WORKER_DEBUG)
			printf("adding lost worker's work unit back to the pool!\n");
		// clock_t start, diff;
		// if (METRICS && MUTEX_METRIC)
		// {
		// 	start = clock();
		// }
		pthread_mutex_lock(&dispatcher->workunits_mutex);
		dispatcher->worker_cnt--;
		pthread_mutex_unlock(&dispatcher->workunits_mutex);
		// if (METRICS && MUTEX_METRIC)
		// {
		// 	diff = clock() - start;
		// 	int msec = diff * 1000 / CLOCKS_PER_SEC;
		// 	G_movelist_locked += msec%1000;
		// }
		while (queue_count(worker->workunit_queue) > 0)
		{
			queue_enqueue(&dispatcher->bundles, queue_create_new(queue_pop(&worker->workunit_queue)));
			sem_post(dispatcher->start_sending);
		}
	}
	worker->workunit_queue = NULL;
	if (DEBUG && WORKER_DEBUG)
		printf("removing worker link!\n");
	// clock_t start, diff;
	// if (METRICS && MUTEX_METRIC)
	// {
	// 	start = clock();
	// }
	// if (METRICS && MUTEX_METRIC)
	// {
	// 	diff = clock() - start;
	// 	int msec = diff * 1000 / CLOCKS_PER_SEC;
	// 	G_removeworker_locked += msec%1000;
	// }
	pthread_mutex_lock(&dispatcher->workers_queue->mutex);
	remove_link(&dispatcher->workers_queue->first, worker);
	pthread_mutex_unlock(&dispatcher->workers_queue->mutex);
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
	if (DEBUG && MSG_DEBUG && MSG_DETAILS_DEBUG)
		printf("handling request!\n");
	if (msg.id == WORK_UNIT_REQUEST)	
		handle_workunit_req(dispatcher, worker, msg);
	else if (msg.id == WORK_UNIT_DONE)
	{
		if (METRICS && WORKER_TIME_METRIC)
		{
			G_worker_calcs += time(NULL) - worker->w_calc_time;
		}
		handle_worker_done_msg(dispatcher, worker, msg);
		worker->w_calc_time = 0;
	}
	else
		printf("invalid msg id: %d\n", msg.id);
}

void 		print_worker_fds(t_dispatcher *dispatcher)
{
	t_lst	*head = 0;

	// clock_t start, diff;
	// if (METRICS && MUTEX_METRIC)
	// {
	// 	start = clock();
	// }
	//pthread_mutex_lock(&dispatcher->worker_list_mutex);
	// if (METRICS && MUTEX_METRIC)
	// {
	// 	diff = clock() - start;
	// 	int msec = diff * 1000 / CLOCKS_PER_SEC;
	// 	G_printfds_locked += msec%1000;
	// }
	pthread_mutex_lock(&dispatcher->workers_queue->mutex);
	head = dispatcher->workers_queue->first;
	 printf("-------------\n");
  	 while (head)
 	 {
	 	printf("(%p)worker: %d -> (%p)\n", head,
	 		((t_worker*)(head->data))->socket.fd, head->next);
  	 	head = head->next;
 	 }
	printf("-------------\n");
	pthread_mutex_unlock(&dispatcher->workers_queue->mutex);
}

void		*handle_worker_connection(void *input)
{
	t_thread_handler	*params;
	t_lst				*worker_link;
	t_worker			*worker;
	t_msg				msg;

	signal(SIGPIPE, SIG_IGN);
	if (DEBUG && WORKER_DEBUG)
		printf("Launched worker network handler thread!\n");
	//if (DEBUG && WORKER_DEBUG)
	  //  print_worker_fds(params->dispatcher);
	params = (t_thread_handler *)input;
	worker_link = params->worker;
	worker = (t_worker *)worker_link->data;
	worker->active = 1;
	//send_worker_msg(worker, new_message(WORK_UNITS_READY, 1, " "));
	while (worker->active)
	{
	  //if (DEBUG && WORKER_DEBUG)
		  //print_worker_fds(params->dispatcher);
		msg = get_msg(worker->socket.fd);
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

void		launch_worker_event_threads(t_dispatcher *dispatcher)
{
	t_lst				*head;
	t_worker			*cur_worker;
	t_thread_handler	*param;

	// clock_t start, diff;
	// if (METRICS && MUTEX_METRIC)
	// {
	// 	start = clock();
	// }
	//pthread_mutex_lock(&dispatcher->worker_list_mutex);
	// if (METRICS && MUTEX_METRIC)
	// {
	// 	diff = clock() - start;
	// 	int msec = diff * 1000 / CLOCKS_PER_SEC;
	// 	G_workerevent_locked += msec%1000;
	// }
	pthread_mutex_lock(&dispatcher->workers_queue->mutex);
	head = dispatcher->workers_queue->first;
	while (head)
	{	
		cur_worker = (t_worker *)head->data;
		if (cur_worker->tid == 0)
		{
			param = new_thread_handler(dispatcher, head);
			cur_worker->tid = calloc(1, sizeof(pthread_t));
			pthread_create(cur_worker->tid, NULL, handle_worker_connection,
				param);
		}
		head = head->next;
	}
	pthread_mutex_unlock(&dispatcher->workers_queue->mutex);
}

int		timeout_progressbar(t_dispatcher *dispatcher)
{
	int timeout = 60;
	printf("\rPress \x1b[32m[ENTER] \x1b[0mto start dispatching workunits\n");
	write(1, "[2K", 4);
	write(1, "\rWaiting for workers to connect...\n", 35);
	while (queue_count(dispatcher->workers_queue) > 0)
	{
		sleep(5);
		if (--timeout == 0)
		{
			printf("Timeout reached, Simulation aborted!");
			return (-1);
		}
	}
	return (0);
}

void		launch_simulation(t_dispatcher *dispatcher)
{
	t_worker			*cur_worker;
	t_thread_handler	*param;
	t_lst				*head;	

	if (sem_unlink("/exit") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	if (sem_unlink("/sender_thread") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	if (sem_unlink("/sender_limit") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	dispatcher->start_sending = sem_open("/sender_thread", O_CREAT, 0777, 0);
	if (dispatcher->start_sending == SEM_FAILED)
		printf("start_sending sem open failed with %d\n", errno);
	dispatcher->exit_sem = sem_open("/exit", O_CREAT, 0777, 0);
	if (dispatcher->exit_sem == SEM_FAILED)
		printf("sem3 open failed with %d\n", errno);

	setup_async_file(dispatcher);
	if (DEBUG)
		printf("begin launch_simulation\n");
	if (timeout_progressbar(dispatcher) == -1)
		return ;
	while (getchar() != 10)
		;
	dispatcher->is_running = 1;
	G_tick_start = time(NULL);
	dispatcher->sender_limit = sem_open("/sender_limit", O_CREAT, 0777, dispatcher->worker_cnt);
	if (dispatcher->sender_limit == SEM_FAILED)
		printf("start_sending sem open failed with %d\n", errno);
	setup_async_file(dispatcher);
	start_sender_threads(dispatcher, dispatcher->worker_cnt);
	launch_worker_event_threads(dispatcher);
	divide_dataset(dispatcher);
	//sem_post(dispatcher->start_sending);
	printf("Simulation Started\n");
	if (sem_wait(dispatcher->exit_sem) < 0)
		printf("sem_wait failed with err:%d\n", errno);
	if (sem_unlink("/sender_limit") && DEBUG)
	 	printf("sem_unlink err %d\n", errno);
	if (sem_unlink("/sender_thread") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	if (sem_unlink("/exit") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	printf("dispatcher done, good bye\n");
}
