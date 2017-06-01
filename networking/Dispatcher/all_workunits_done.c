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
		printf("\nall workunits done for this tick\n");
	// Output tick to file
	save_output(dispatcher, dispatcher->name);
	// Clear Work Units
	//clear_work_units(&dispatcher->workunits);
	dispatcher->ticks_done++;
	if (METRICS)
	{
		printf("%d workers completed %d workunits totalling %ld MB\n", dispatcher->worker_cnt, dispatcher->workunits_done, G_workunit_size / (1024 * 1024));
		G_workunit_size = 0;
		printf("worker units took %f seconds\n", G_worker_calcs);
		printf("worker units took an avg of %f seconds\n", G_worker_calcs/(double)dispatcher->workunits_done);
		printf("workers were waiting on locks for %d milliseconds\n", G_locked);
	}
	G_worker_calcs = 0;
	double tick_time = time(NULL) - tick_start;
	if (TPM_METRIC)
	{
		printf("this tick took: %f seconds\n", tick_time);
		printf("average ticks per minute: %f\n", (double)dispatcher->ticks_done / ((time(NULL) - G_total_time) / 60.0f));
		G_avg_ten_ticks_min += (double)dispatcher->ticks_done / ((time(NULL) - G_total_time) / 60.0f);
		printf("average workunits per minute: %f\n\n", (double)dispatcher->workunits_done / ((time(NULL) - G_total_time) / 60.0f));
		G_avg_ten_wu_min += (double)dispatcher->workunits_done / ((time(NULL) - G_total_time) / 60.0f);
		if (dispatcher->ticks_done % 10 == 0 && dispatcher->ticks_done != 0)
		{
			printf("Average ticks/min for last 10 ticks %f\n", G_avg_ten_ticks_min / 10);
			printf("Average workunits/min for last 10 ticks %f\n", G_avg_ten_wu_min / 10);
			G_avg_ten_ticks_min = 0;
			G_avg_ten_wu_min = 0;
		}
	}
	tick_start = time(NULL);
	dispatcher->workunits_done = 0;
	free(dispatcher->cells);
	//move new_dataset to dataset
	free(dispatcher->dataset->particles);
	free(dispatcher->dataset);
	dispatcher->dataset = dispatcher->new_dataset;
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
		exit(1);
	}
}
