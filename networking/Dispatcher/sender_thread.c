/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sender_thread.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/10 17:44:34 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/10 18:41:17 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	*sender_thread(void *input)
{
	t_thread_handler	*params;
	t_dispatcher		*dispatcher;
	t_lst				*worker_link;
	t_worker			*worker;
	t_bundle			*bundle;
	t_msg				msg;

	signal(SIGPIPE, SIG_IGN);
	if (DEBUG && WORKER_DEBUG)
		printf("Launched worker sender thread!\n");
	params = (t_thread_handler *)input;
	dispatcher = params->dispatcher;
	worker_link = params->worker;
	worker = (t_worker *)worker_link->data;
	worker->active = 1;
	while (worker->active)
	{
		if (sem_wait(worker->start_sending) < 0)
			printf("SEND- sem_wait failed with err:%d\n", errno);
		while (worker->sending && queue_count(&dispatcher->bundles) > 0)
		{
			if (DEBUG)
				printf("SEND- sending work unit\n");
			bundle = queue_pop(&dispatcher->bundles);
			queue_enqueue(&worker->workunit_queue, queue_create_new(bundle));
			if (bundle)
			{
				send_bundle(worker, bundle, dispatcher->cells);
				if (METRICS && WORKER_TIME_METRIC)
					worker->w_calc_time = time(NULL);
				G_sent_wu++;
			}
			else
				printf("bundle to send is null!\n");
		}
		worker->sending = 0;
	}
	free(params);
	free(worker->sender_tid);
	free(worker->start_sending);
	if (DEBUG && sem_unlink("/worker_sender_thread"))
		printf("sem_unlink err %d\n", errno);
	if (DEBUG && WORKER_DEBUG)
		printf("killing this sender thread...\n");
	return (0);
}

void	start_sender_thread(t_dispatcher *disp, t_lst *worker_link)
{ 
	t_thread_handler	*params;
	t_worker			*worker;

	params = new_thread_handler(disp, worker_link);
	worker = (t_worker *)worker_link->data;
	if (DEBUG && sem_unlink("/worker_sender_thread"))
		printf("sem_unlink err %d\n", errno);
	worker->start_sending = sem_open("/worker_sender_thread", O_CREAT, 0777, 0);
	if (worker->start_sending == SEM_FAILED)
		printf("start_sending sem open failed with %d\n", errno);
	worker->sender_tid = calloc(1, sizeof(pthread_t));
	pthread_create(worker->sender_tid, NULL, sender_thread, params);
}
