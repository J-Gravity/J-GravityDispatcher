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

#include "standalone.h"

static void duplicate_dataset(t_standalone *sim)
{
	free(sim->cells);
	free(sim->dataset->positions);
	free(sim->dataset->velocities);
	//free(sim->dataset->particles);
	free(sim->dataset);
	sim->dataset = sim->new_dataset;
	sim->new_dataset = (t_dataset *)calloc(1, sizeof(t_dataset));
	sim->new_dataset->positions = (cl_float4 *)calloc(sim->dataset->particle_cnt, sizeof(cl_float4));
	sim->new_dataset->velocities = (cl_float4 *)calloc(sim->dataset->particle_cnt, sizeof(cl_float4));
	sim->new_dataset->particle_cnt = sim->dataset->particle_cnt;
	sim->new_dataset->max_scale = sim->dataset->max_scale;
}

static void	*simulation_thread(void *param)
{
	t_standalone	*sim;

	sim = (t_standalone *)param;
	while (sim->ticks_done < sim->ticks_cnt)
	{
		if (sim->ticks_done > 0)
		{
			duplicate_dataset(sim);
			fclose(sim->fp);
			//fclose(sim->snap);
		}
		//setup_snap_file(sim);
		setup_async_file(sim);
		sim->workunits_done = 0;
		divide_dataset(sim);
		//wait for last tick to complete
		if (sem_wait(sim->next_tick_sem) < 0)
			printf("SIMULATION- sem_wait failed with err:%d\n", errno);
	}
	printf("SIMULATION- exiting simulation thread\n");
	sem_post(sim->exit_sem);
	return (0);
}

void		launch_simulation_thread(t_standalone *sim)
{
	sim->simulation_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(sim->simulation_thread, NULL, simulation_thread, sim);
}
