/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connect_workers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 21:10:00 by scollet           #+#    #+#             */
/*   Updated: 2017/05/18 16:52:03 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <unistd.h>
#include <pthread.h>

void	*connect_worker_thread(void *param)
{
	t_lst					*new_link;
	t_worker				*new_worker;
	t_socket				*sin;
	t_dispatcher			*dispatcher;
	struct sockaddr_storage	their_addr;
	int						fd;

	dispatcher = (t_dispatcher *)param;
	while (dispatcher->is_connect)
	{
		sin = calloc(1, sizeof(t_socket));
		printf("1\n");
		fd = accept(sin->fd, (struct sockaddr *)&(sin->addr.sin_addr), &(sin->addrlen));
		printf("worker connected fd: %d\n", fd);
		new_link = calloc(1, sizeof(t_lst));
		new_link->data = calloc(1, sizeof(t_worker));
		printf("3\n");
		new_link->next = dispatcher->workers;
		printf("4\n");
		dispatcher->workers = new_link;
		printf("5\n");
		new_worker = (t_worker *)new_link->data;
		printf("6\n");
		new_worker->socket.fd = fd;
		printf("7\n");
		new_worker->tid = 0;
		printf("8\n");
		if (new_worker->socket.fd == -1)
		{
			printf("worker accept call failed\n");
			return (0);
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
