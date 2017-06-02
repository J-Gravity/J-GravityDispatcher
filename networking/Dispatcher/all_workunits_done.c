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
	if (METRICS)
	{
		printf("%d workers completed %d workunits totalling %ld MB\n", dispatcher->worker_cnt, dispatcher->workunits_done, G_workunit_size / (1024 * 1024));
		G_workunit_size = 0;
		printf("Total workunits took %f seconds\n", G_worker_calcs);
		printf("The average workunit took %f seconds\n", G_worker_calcs/(double)dispatcher->workunits_done);
		printf("workers were waiting on locks for %d milliseconds\n", G_locked);
	}
	G_worker_calcs = 0;
	double tick_time = (time(NULL) - G_tick_start);
	if (TPM_METRIC)
	{
		printf("this tick took: %.0f seconds\n", tick_time);
		printf("average ticks per minute: %.2f\n", (double)dispatcher->ticks_done / tick_time / 60);
		G_last_ten_ticks_permin += (double)dispatcher->ticks_done / tick_time;
		printf("average workunits per minute: %.2f\n\n", (double)dispatcher->workunits_done / tick_time / 60);
		G_last_ten_workunits_permin += (double)dispatcher->workunits_done / tick_time;
		if (dispatcher->ticks_done % 10 == 0 && dispatcher->ticks_done != 0)
		{
			printf("\n\x1b[32mAverage ticks/sec for last 10 ticks %.2f\n", G_last_ten_ticks_permin * 60 / 10);
			printf("Average workunits/sec for last 10 ticks %.2f\n\n", G_last_ten_workunits_permin * 60 / 10);
			printf("\x1b[0m");
			G_last_ten_ticks_permin = 0;
			G_last_ten_workunits_permin = 0;
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
		printf("total worker calc time: %f seconds\n", (time(NULL) - G_total_time));
		printf("simulation complete\n");
		close(dispatcher->sin.fd);
		exit(1);
	}
}
