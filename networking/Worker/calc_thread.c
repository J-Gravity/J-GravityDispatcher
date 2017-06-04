/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   calc_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:29:42 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/03 16:53:43 by ssmith           ###   ########.fr       */
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
		if (worker->todo_work->count > 0)
		{
			printf("worker_todo\n");
		if (DEBUG)
			printf("calculating work unit\n");
		workunit = queue_pop(&worker->todo_work);
		printf("c-1\n");
		workunit = do_workunit(workunit);
		printf("c0\n");
		queue_enqueue(&worker->completed_work, queue_create_new(*workunit));
		printf("c1\n");
		worker->completed_work->count++;
		printf("c2\n");
		worker->todo_work->count--;
		printf("calc finished loop\n");
		}
	}
	return (0);
}

void launch_calculation_thread(t_worker *worker)
{
	worker->calc_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->calc_thread, NULL, calc_thread, worker);
}
