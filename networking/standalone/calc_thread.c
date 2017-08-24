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

#include "standalone.h"

static void *calc_thread(void *param)
{
	t_standalone *sim;
	t_workunit  *workunit;

	sim = (t_standalone *)param;
	while (1)
	{
		if (sem_wait(sim->calc_thread_sem) < 0)
		    printf("CALC- sem_wait failed with err:%d\n", errno);
		workunit = queue_pop(&sim->todo_work);
		if(!workunit)
			printf("WU was NULL!!\n");
		do_workunit(sim, workunit);
		queue_enqueue(&sim->completed_work, queue_create_new(workunit));
    	sem_post(sim->integration_thread_sem);
	}
	return (0);
}

void launch_calculation_thread(t_standalone *sim)
{
	//this should launch GPU_COUNT threads
	sim->calc_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(sim->calc_thread, NULL, calc_thread, sim);
}
