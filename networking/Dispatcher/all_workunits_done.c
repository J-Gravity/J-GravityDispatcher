	/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   all_workunits_done.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/16 21:48:12 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/31 14:21:41 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	all_workunits_done(t_dispatcher *dispatcher)
{
	if (DEBUG)
		printf("all workunits done for this tick\n");
	// Output tick to file
	save_output(dispatcher, dispatcher->name);
	// Clear Work Units
	//clear_work_units(&dispatcher->workunits);
	dispatcher->ticks_done++;
	double tick_time = (time(NULL) - G_tick_start);
	G_total_time += tick_time;
	if (METRICS)
	{
		printf("--------------------------------Tick %d---------------------------\n", dispatcher->ticks_done);
		printf("%d workers completed %d workunits totalling %ld MB\n", dispatcher->worker_cnt, dispatcher->workunits_done, G_workunit_size / (1024 * 1024));
		G_workunit_size = 0;
		printf("Total workunits took %f seconds\n", G_worker_calcs);
		printf("The average workunit took %f seconds\n", G_worker_calcs / (double)dispatcher->workunits_done);
		printf("workers were waiting on locks for %d milliseconds\n", G_locked);
	}
	G_worker_calcs = 0;
	if (TPM_METRIC)
	{
		printf("this tick took: %.0f seconds\n", tick_time);
		printf("This tick ran at %.2f tick/min\n", 60.0 / tick_time);
		printf("These workunits ran at %.2f workunits/min\n\n", 60 * ((double)dispatcher->workunits_done / tick_time));
		G_total_workunit_cnt += dispatcher->workunits_done;
		if (dispatcher->ticks_done % dispatcher->ticks_cnt == 0 && dispatcher->ticks_done != 0)
		{
			printf("\n\x1b[32mAverage ticks/min %.2f\n",  dispatcher->ticks_cnt / (G_total_time / 60));
			printf("Average workunits/min %.2f\n\n", 60 * (G_total_workunit_cnt / G_total_time));
			printf("\x1b[0m");
		}
	}
	dispatcher->workunits_done = 0;
	free(dispatcher->cells);
	//move new_dataset to dataset
	free(dispatcher->dataset->particles);
	free(dispatcher->dataset);
	dispatcher->dataset = dispatcher->new_dataset;
	G_tick_start = time(NULL);
	dispatcher->new_dataset = (t_dataset *)calloc(1, sizeof(t_dataset));
	dispatcher->new_dataset->particles = calloc(dispatcher->dataset->particle_cnt, sizeof(t_body));
	dispatcher->new_dataset->particle_cnt = dispatcher->dataset->particle_cnt;
	dispatcher->new_dataset->max_scale = dispatcher->dataset->max_scale;
	if (dispatcher->ticks_done < dispatcher->ticks_cnt)
	{
		// re-Divide the dataset into work units
		divide_dataset(dispatcher);
		// Inform all workers work units are ready
		pthread_mutex_lock(&dispatcher->worker_list_mutex);
		if (DEBUG && MUTEX_DEBUG)
			printf("worker list mutex locked!\n");
		broadcast_worker_msg(dispatcher->workers, new_message(WORK_UNITS_READY, 0, ""));
		pthread_mutex_unlock(&dispatcher->worker_list_mutex);
		if (DEBUG && MUTEX_DEBUG)
			printf("worker list mutex unlocked!\n");

	}
	else
	{
		printf("total worker calc time: %f seconds\n", G_total_time);
		printf("simulation complete\n");
		close(dispatcher->sin.fd);
		exit(1);
	}
}
