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

#include "standalone.h"

void integrate_WU_results(t_standalone *sim, t_tree *old_cell, t_workunit *new_WU)
{
	cl_float4 *pos_start = (cl_float4 *)(old_cell->positions - sim->dataset->positions + sim->new_dataset->positions);
	cl_float4 *vel_start = (cl_float4 *)(old_cell->velocities - sim->dataset->velocities + sim->new_dataset->velocities);
	memcpy(pos_start, new_WU->N, new_WU->n_count * sizeof(cl_float4));
	memcpy(vel_start, new_WU->V, new_WU->n_count * sizeof(cl_float4));
	async_save(sim, old_cell->positions - sim->dataset->positions, new_WU);
}

static void delete_workunit(t_workunit *w)
{
	free(w->N);
	free(w->V);
	free(w->M);
	clReleaseEvent(w->done[0]);
	clReleaseEvent(w->done[1]);
}

static void *integration_thread(void *param)
{
	t_standalone *sim;
	t_tree		*local_cell;
	t_workunit	*workunit;

	sim = (t_standalone *)param;
    while (1)
    {
	    if (sem_wait(sim->integration_thread_sem) < 0)
		    printf("SEND- sem_wait failed with err:%d\n", errno);

        workunit = queue_pop(&sim->completed_work);
		local_cell = sim->cells[workunit->idx];
		clWaitForEvents(2, workunit->done);
		integrate_WU_results(sim, local_cell, workunit);
		pthread_mutex_lock(&sim->workunits_done_mutex);
		sim->workunits_done++;
		if (sim->workunits_done == sim->cell_count)
		{
			printf("workunits done\n");
			sim->ticks_done++;
			fflush(sim->fp);
			sem_post(sim->next_tick_sem);
		}
		pthread_mutex_unlock(&sim->workunits_done_mutex);
		delete_workunit(workunit);
		free(workunit);
    }
    return (0);
}

#define INTEGRATION_THREADS 1 //ideally this is many but clWaitForEvents seems to hate multithreading

void launch_integration_thread(t_standalone *sim)
{
	pthread_t *threads = calloc(INTEGRATION_THREADS, sizeof(pthread_t));
	for (int i = 0; i < INTEGRATION_THREADS; i++)
		pthread_create(&threads[i], NULL, integration_thread, sim);
}
