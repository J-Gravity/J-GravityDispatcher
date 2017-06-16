/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   calc_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:29:42 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/15 01:54:57 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void *calc_thread(void *param)
{
	t_worker	*worker;
	t_workunit  *workunit;

	worker = (t_worker *)param;
	while (1)
	{
		if (sem_wait(worker->calc_thread_sem) < 0)
		    printf("CALC- sem_wait failed with err:%d\n", errno);
		if (DEBUG)
			printf("CALC- calculating work unit\n");
		clock_t calc_time = time(NULL);
		workunit = queue_pop(&worker->todo_work);
		do_workunit(workunit);
		if (worker->todo_work->count == 0)
		{
			workunit->is_last = 1;
			sem_post(worker->ready_for_bundle);
			printf("calc queue emptied\n");
		}
		if (DEBUG)
			printf("CALC- is last workunit of a bundle: %d\n", workunit->is_last);
		if (DEBUG)
			printf("CALC- finished calculating work unit\n");
		queue_enqueue(&worker->completed_work, queue_create_new(workunit));
		sem_post(worker->sender_thread_sem);
		G_total_calc_time += time(NULL) - calc_time;
	}
	return (0);
}

void launch_calculation_thread(t_worker *worker)
{
	worker->calc_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->calc_thread, NULL, calc_thread, worker);
}
