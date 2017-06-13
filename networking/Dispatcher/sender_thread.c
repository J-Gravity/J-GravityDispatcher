/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sender_thread.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/10 17:44:34 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/12 23:08:51 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	*sender_thread(void *input)
{
	t_dispatcher		*dispatcher;
	t_worker			*worker;
	t_bundle			*bundle;

	signal(SIGPIPE, SIG_IGN);
	if (DEBUG)
		printf("Launching a sender thread!\n");
	dispatcher = (t_dispatcher *)input;
	while (dispatcher->is_running)
	{
		if (sem_wait(dispatcher->start_sending) < 0)
			printf("SEND- sem_wait failed with err:%d\n", errno);
		while (queue_count(dispatcher->bundles) > 0)
		{
			worker = queue_pop(&dispatcher->workers_queue);
			if (worker)
			{
				bundle = queue_pop(&dispatcher->bundles);
				if (bundle)
				{
					queue_enqueue(&worker->workunit_queue, queue_create_new(bundle));
					if (DEBUG)
						printf("bundle sent to worker %d\n", worker->socket.fd);
					send_bundle(worker, bundle, dispatcher->cells);
					if (DEBUG)
						printf("done sending bundle sent to worker %d\n", worker->socket.fd);
				}
				else
				{
					printf("bundle to send is null!\n");
					break ;
				}
			}
			else
			{
				printf("NULL from poping the worker queue!\n");
				break ;
			}
			queue_enqueue(&dispatcher->workers_queue, queue_create_new(worker));
		}
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
