/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connect_workers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 21:10:00 by scollet           #+#    #+#             */
/*   Updated: 2017/05/29 20:48:20 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <unistd.h>
#include <pthread.h>

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
			continue;
		new_link = calloc(1, sizeof(t_lst));
		new_link->data = calloc(1, sizeof(t_worker));
		new_link->next = NULL;

		clock_t start = clock(), diff;
		pthread_mutex_lock(&dispatcher->worker_list_mutex);
		diff = clock() - start;
		int msec = diff * 1000 / CLOCKS_PER_SEC;
		G_locked = msec/1000 + msec%1000;
		head = dispatcher->workers;
		if (head)
		{
			while (head)
			{
				if (head->next == NULL)
					break;
				head = head->next;
			}
			head->next = new_link;
		}
		else
		{
			dispatcher->workers = new_link;
		}
		new_worker = (t_worker *)new_link->data;
		new_worker->socket.fd = fd;
		new_worker->tid = 0;
		pthread_mutex_unlock(&dispatcher->worker_list_mutex);
		if (new_worker->socket.fd == -1)
			return (0);
		if (dispatcher->is_running && fd != -1)
		{
			printf("new_link(%p) worker(%p)\n", new_link, new_link->data);
			make_new_event_thread(dispatcher, new_link);
		}
	}
	return (0);
}

void  connect_workers(t_dispatcher *dispatcher, t_lst **workers)
{
	pthread_t	*worker_conn_thr;

	worker_conn_thr = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker_conn_thr, NULL, connect_worker_thread, dispatcher);
	return ;
}
