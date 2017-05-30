	/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   all_workunits_done.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/16 21:48:12 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/30 15:38:45 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	all_workunits_done(t_dispatcher *dispatcher)
{
	printf("all workunits done!\n");
	// Output tick to file
	save_output(dispatcher, dispatcher->name);
	// Clear Work Units
	//clear_work_units(&dispatcher->workunits);
	// Reset work units done
	printf("there were %d workunits totalling %d MB\n", dispatcher->workunits_done, G_workunit_size);
	printf("worker units took %ld seconds\n", G_worker_calcs);
	printf("worker units took an avg of %ld seconds\n", G_worker_calcs/dispatcher->workunits_done);
	printf("workers were waiting on locks for %d milliseconds\n", G_locked);
	G_worker_calcs = 0;
	printf("10 second sleep\n");
	sleep(10);
	dispatcher->workunits_done = 0;
	dispatcher->ticks_done += 1;
	free(dispatcher->cells);
	//printf("about to free dataset stuff\n");
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
				printf("worker list mutex locked!\n");
		broadcast_worker_msg(dispatcher->workers, new_message(WORK_UNITS_READY, 0, ""));
		pthread_mutex_unlock(&dispatcher->worker_list_mutex);
				printf("worker list mutex unlocked!\n");
	}
	else
	{
		//simulation complete
		printf("simulation complete\n");
		exit(1);
	}
}
