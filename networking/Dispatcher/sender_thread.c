/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sender_thread.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/10 17:44:34 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/15 19:34:48 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	*sender_thread(void *input)
{
	t_dispatcher		*dispatcher;
	t_worker			*worker;
	t_lst			*worker_link;
	t_bundle			*bundle;

	signal(SIGPIPE, SIG_IGN);
	if (DEBUG)
		printf("Launching a sender thread!\n");
	dispatcher = (t_dispatcher *)input;
	while (dispatcher->is_running)
	{
		if (sem_wait(dispatcher->start_sending) < 0)
			printf("SEND- sem_wait failed with err:%d\n", errno);
		//printf("cleared start sending semaphore\n");
		pthread_mutex_lock(&dispatcher->sender_thread_mutex);
		if (queue_count(dispatcher->bundles) > 0)
		{
			worker_link = queue_pop_link(&dispatcher->workers_queue);
			if (!worker_link || !worker_link->data)
			{
				//printf("NULL from popping the worker queue!\n");
				sem_post(dispatcher->start_sending);
				pthread_mutex_unlock(&dispatcher->sender_thread_mutex);
				continue ;
			}
			worker = worker_link->data;
			bundle = queue_pop(&dispatcher->bundles);
			pthread_mutex_unlock(&dispatcher->sender_thread_mutex);
			if (bundle)
			{
				queue_enqueue(&worker->workunit_queue, queue_create_new(bundle));
				if (DEBUG)
					printf("sending bundle %d to worker %d\n", bundle->id, worker->socket.fd);
				if (worker->active == 0)
					{
					printf("trying to send a bundle to an inactive worker\n");
					}
				send_bundle(worker, bundle, dispatcher->cells);
				if (DEBUG)
					printf("done sending bundle %d to worker %d\n", bundle->id, worker->socket.fd);
			}
			queue_enqueue(&dispatcher->workers_queue, worker_link);
		}
		else
			pthread_mutex_unlock(&dispatcher->sender_thread_mutex);
	}
	if (DEBUG)
		printf("killing a sender thread...\n");
	return (0);
}

void	start_sender_threads(t_dispatcher *disp, int count)
{
	disp->sender_threads = (pthread_t **)calloc(count, sizeof(pthread_t *));
	for (int i = 0; i < count; i++)
	{
		disp->sender_threads[i] = (pthread_t *)calloc(1, sizeof(pthread_t));
		pthread_create(disp->sender_threads[i], NULL, sender_thread, disp);
	}
}
