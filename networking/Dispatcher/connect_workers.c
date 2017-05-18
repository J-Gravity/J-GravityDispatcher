/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connect_workers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 21:10:00 by scollet           #+#    #+#             */
/*   Updated: 2017/05/17 23:43:32 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <unistd.h>
#include <pthread.h>

void	*connect_worker_thread(void *param)
{
	t_lst			*new_worker;
	t_socket		*server_sock;
	t_dispatcher	*dispatcher;
	dispatcher = (t_dispatcher *)param;

	while (dispatcher->is_connect)
	{
		new_worker = calloc(1, sizeof(t_worker));
		new_worker->next = dispatcher->workers;
		dispatcher->workers = new_worker;
		new_worker->data = (t_worker *)calloc(1, sizeof(t_worker));
		((t_worker *)new_worker)->socket.fd = accept(server_sock->fd, (struct sockaddr *)&server_sock->addr, server_sock->addrlen);
		if (((t_worker *)new_worker)->socket.fd == -1)
			return (0);
		((t_worker *)new_worker)->tid = 0;
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
