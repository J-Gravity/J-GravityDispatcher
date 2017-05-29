/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connect_workers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 21:10:00 by scollet           #+#    #+#             */
/*   Updated: 2017/05/29 14:36:34 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <unistd.h>
#include <pthread.h>


void	add_link(t_lst **list, t_lst *new_link)
{
	new_link->next = *list;
	*list = new_link;
}

void	add_link_end(t_lst **list, t_lst *new_link)
{
	t_lst	*head;

	head = *list;
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
		*list = new_link;
	}
}

void 	add_worker(t_dispatcher *dispatcher, int fd)
{
	t_lst		*new_link;
	t_worker	*new_worker;

	new_link = (t_lst *)calloc(1, sizeof(t_lst));
	new_link->data = calloc(1, sizeof(t_worker));
	new_link->next = NULL;
	new_worker = (t_worker *)new_link->data;
	new_worker->socket.fd = fd;
	pthread_mutex_lock(&dispatcher->worker_list_mutex);
	if (DEBUG && MUTEX_DEBUG)
		printf("worker list mutex locked!\n");
	add_link(&dispatcher->workers, new_link);
	pthread_mutex_unlock(&dispatcher->worker_list_mutex);
	if (DEBUG && MUTEX_DEBUG)
		printf("worker list mutex unlocked!\n");
	if (dispatcher->is_running)
	{
		if (DEBUG)
		{
			printf("launching event thread from connect workers\n");
			printf("new_link(%p) worker(%p)\n", new_link, new_worker);
		}
		make_new_event_thread(dispatcher, new_link);
	}
}

void	*connect_worker_thread(void *param)
{
	t_dispatcher			*dispatcher;
	struct sockaddr_storage	their_addr;
	int						fd;

	dispatcher = (t_dispatcher *)param;
	while (dispatcher->is_connect)
	{
		fd = accept(dispatcher->sin.fd,
			(struct sockaddr *)&(dispatcher->sin.addr.sin_addr),
			&(dispatcher->sin.addrlen));
		if (fd == -1)
		{
			printf("worker accept call failed with err %d\n", errno);
			return (0);
		}
		if (DEBUG)
			printf("worker connected fd: %d\n", fd);
		add_worker(dispatcher, fd);
		if (DEBUG)
			printf("finished with connect worker\n");
	}
	if (DEBUG)
		printf("connect worker thread terminated!"
			"No more workers will be added to the pool\n");
	return (0);
}

void	connect_workers(t_dispatcher *dispatcher, t_lst **workers)
{
	pthread_t	*worker_conn_thr;

	worker_conn_thr = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker_conn_thr, NULL, connect_worker_thread, dispatcher);
	return ;
}
