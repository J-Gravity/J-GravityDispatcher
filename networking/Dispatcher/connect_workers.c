/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connect_workers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 21:10:00 by scollet           #+#    #+#             */
/*   Updated: 2017/06/24 22:43:24 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <unistd.h>
#include <pthread.h>

void	configure_worker_settings(t_dispatcher *dispatcher, t_worker *worker)
{
	t_msg	settings_msg;
	t_msg	response;

	settings_msg = serialize_settings(dispatcher);
	send_worker_msg(worker, settings_msg);
	free(settings_msg.data);

	response = get_msg(worker->socket.fd);
	if (response.id != SETTINGS_APPLIED)
		printf("ERROR: Worker Failed to respond to SETTINGS MSG\n");
	else
		printf("Worker settings applied successfully!!\n");
}

void	*connect_worker_thread(void *param)
{
	t_lst					*head;
	t_lst					*new_link;
	t_worker				*new_worker;
	t_dispatcher			*dispatcher;
	struct sockaddr_storage	their_addr;
	int						fd;

	dispatcher = (t_dispatcher *)param;
	while (dispatcher->is_connect)
	{
		fd = accept(dispatcher->sin.fd, (struct sockaddr *)&(dispatcher->sin.addr.sin_addr), &(dispatcher->sin.addrlen));
		if (fd == 0)
		{
			printf("accept returned 0!");
			continue;
		}
		if (fd == -1)
		{
			if (DEBUG && WORKER_DEBUG)
				printf("worker accept call failed\n");
			return (0);
		}
		printf("%d worker", dispatcher->worker_cnt + 1);
		if (dispatcher->worker_cnt + 1 == 1)
		  printf(" is connected! - fd: %d\n", fd);
		else
		  printf("s are connected! - fd: %d\n" ,fd);
		new_link = calloc(1, sizeof(t_lst));
		new_link->data = calloc(1, sizeof(t_worker));
		new_link->next = NULL;

		pthread_mutex_lock(&dispatcher->worker_list_mutex);
		dispatcher->worker_cnt++;
		pthread_mutex_unlock(&dispatcher->worker_list_mutex);
		if (DEBUG && MUTEX_DEBUG)
			printf("worker list mutex locked!\n");
		new_worker = (t_worker *)new_link->data;
		new_worker->socket.fd = fd;
		new_worker->tid = 0;
		new_worker->workunit_queue = (t_queue *)calloc(1, sizeof(t_queue));
		pthread_mutex_init(&new_worker->workunit_queue->mutex, NULL);
		queue_enqueue(&dispatcher->workers_queue, new_link);
		configure_worker_settings(dispatcher, new_worker);
		if (DEBUG && MUTEX_DEBUG)
			printf("worker list mutex unlocked!\n");
		if (dispatcher->is_running)
		{
			if (DEBUG && WORKER_DEBUG)
			{
				printf("launching event thread from connect workers\n");
				printf("new_link(%p) worker(%p)\n", new_link, new_link->data);
			}
			make_new_event_thread(dispatcher, new_link);
		}
		if (DEBUG && WORKER_DEBUG)
			printf("finished with connect worker\n");
	}
	if (DEBUG && WORKER_DEBUG)
		printf("connect worker thread terminated! No more workers will be added to the pool\n");
	return (0);
}

void  connect_workers(t_dispatcher *dispatcher, t_lst **workers)
{
	pthread_t	*worker_conn_thr;

	worker_conn_thr = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker_conn_thr, NULL, connect_worker_thread, dispatcher);
	return ;
}
