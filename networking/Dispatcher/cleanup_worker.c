/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup_worker.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/29 00:42:48 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/29 00:44:12 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void		cleanup_worker(t_dispatcher *dispatcher, t_lst *worker_link)
{
	t_worker	*worker;

	worker = (t_worker *)worker_link->data;
	while (queue_count(worker->workunit_queue) > 0)
	{
		if (DEBUG && WORKER_DEBUG)
			printf("adding lost worker's bundle back to the pool!\n");
		queue_enqueue(&dispatcher->bundles, queue_create_new(queue_pop(&worker->workunit_queue)));
		sem_post(dispatcher->start_sending);
	}
	free(worker->workunit_queue);
	worker->workunit_queue = NULL;
	if (DEBUG && WORKER_DEBUG)
		printf("removing worker link!\n");
	pthread_mutex_lock(&dispatcher->workers->mutex);
	remove_link(&dispatcher->workers->first, worker);
	dispatcher->workers->count--;
	pthread_mutex_unlock(&dispatcher->workers->mutex);
	free(worker_link);
	if (worker->socket.fd)
	{
		close(worker->socket.fd);
		worker->socket.fd = 0;
	}
	free(worker->tid);
	free(worker);
}
