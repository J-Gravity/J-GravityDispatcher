/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   calc_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:29:42 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/03 14:29:04 by ssmith           ###   ########.fr       */
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
		sem_wait(&worker->calc_thread_sem);
		if (DEBUG)
			printf("calculating work unit\n");
		workunit = queue_pop(&worker->todo_work);
		do_workunit(workunit);
		queue_enqueue(&worker->completed_work, queue_create_new(*workunit));
		sem_post(&worker->sender_thread_sem);
	}
	return (0);
}

void launch_calculation_thread(t_worker *worker)
{
	pthread_create(worker->calc_thread, NULL, calc_thread, worker);
}
