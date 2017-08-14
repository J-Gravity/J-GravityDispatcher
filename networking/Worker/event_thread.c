/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   event_thread.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 17:06:01 by cyildiri          #+#    #+#             */
/*   Updated: 2017/08/10 15:26:29 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void duplicate_dataset(t_dispatcher *dispatcher)
{
	free(dispatcher->cells);
	free(dispatcher->dataset->particles);
	free(dispatcher->dataset);
	dispatcher->dataset = dispatcher->new_dataset;
	dispatcher->new_dataset = (t_dataset *)calloc(1, sizeof(t_dataset));
	dispatcher->new_dataset->particles = calloc(dispatcher->dataset->particle_cnt, sizeof(t_body));
	dispatcher->new_dataset->particle_cnt = dispatcher->dataset->particle_cnt;
	dispatcher->new_dataset->max_scale = dispatcher->dataset->max_scale;
}

static void	*simulation_thread(void *param)
{
	t_msg		msg;
	t_worker	*worker;

	worker = (t_worker *)param;
	while (worker->active)
	{
		setup_async_file(dispatcher);
		if (ticks_done > 0)
			duplicate_dataset(dispatcher);
		dispatcher->workunits_done = 0;
		divide_dataset(dispatcher);
		//wait for last tick to complete
		if (sem_wait(worker->next_tick_sem) < 0)
			printf("SIMULATION- sem_wait failed with err:%d\n", errno);
	}
	printf("SIMULATION- exiting simulation thread\n");
	sem_post(worker->exit_sem);
	return (0);
}

void		launch_simulation_thread(t_worker *worker)
{
	worker->active = 1;
	worker->simulation_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->simulation_thread, NULL, simulation_thread, worker);
}
