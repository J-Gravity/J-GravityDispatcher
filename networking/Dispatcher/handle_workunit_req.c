/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_workunit_req.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/05 01:47:21 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_workunit_req(t_dispatcher *dispatcher, t_worker *worker, t_msg msg)
{
	while (dispatcher->workunits->count > 0)
	{
	  clock_t start = clock(), diff;
	  pthread_mutex_lock(&dispatcher->workunits_mutex);
	  if (DEBUG && MUTEX_DEBUG)
	    printf("*work units mutex locked!\n");
	  diff = clock() - start;
	  int msec = diff * 1000 / CLOCKS_PER_SEC;
	  G_handle_locked += msec%1000;
	  queue_enqueue(&worker->workunit_queue,
			queue_create_new(*queue_pop(&dispatcher->workunits)));
	  pthread_mutex_unlock(&dispatcher->workunits_mutex);
	  if (DEBUG && MUTEX_DEBUG)
	    printf("*work units mutex unlocked!\n");
	  G_sent_wu++;
	  if (worker->workunit_queue->last)
	    send_workunit(worker, worker->workunit_queue->last->data);
	  else
	    send_workunit(worker, worker->workunit_queue->first->data);
	}
	free(msg.data);
}
