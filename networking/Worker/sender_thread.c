/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sender_thread.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 19:20:34 by cyildiri          #+#    #+#             */
/*   Updated: 2017/08/10 15:19:49 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void *integration_thread(void *param)
{
	t_worker	*worker;
	t_tree		*local_cell;
	t_workunit	*workunit;
	t_msg		msg;

	worker = (t_worker *)param;
    while (1)
    {
	    if (sem_wait(worker->integration_thread_sem) < 0)
		    printf("SEND- sem_wait failed with err:%d\n", errno);
		clock_t send_time = time(NULL);
        workunit = queue_pop(&worker->completed_work);
		local_cell = dispatcher->cells[complete_WU.id];
		integrate_WU_results(dispatcher, local_cell, workunit);
		free(workunit->local_bodies);
		free(workunit);
		dispatcher->workunits_done++;
		if (workunits_done == workunits_cnt)
		{
			dispatcher->ticks_done++;
			if (dispatcher->ticks_done < dispatcher->ticks_cnt)
			{
				//fire the nextTick semaphore
				sem_post(dispatcher->next_tick_sem);
			}
		}
		G_total_send_time += time(NULL) - send_time;
    }
    return (0);
}

void launch_integration_thread(t_worker *worker)
{
	worker->integration_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->integration_thread, NULL, integration_thread, worker);
}
