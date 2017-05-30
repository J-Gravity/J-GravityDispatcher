/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_workunit_req.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/29 20:51:40 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_workunit_req(t_dispatcher *dispatcher, t_worker *worker, t_msg msg)
{
	t_lst	*delete_me;
	clock_t start = clock(), diff;
	pthread_mutex_lock(&dispatcher->workunits_mutex);
	diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	printf("Handle_workunit_req WAITED on lock for %d seconds %d milliseconds\n", msec/1000, msec%1000);
	G_locked = msec%1000;
	pthread_mutex_lock(&dispatcher->workunits_mutex);
	if (DEBUG && MUTEX_DEBUG)
		printf("*work units mutex locked!\n");
	if (dispatcher->workunits)
	{
		worker->workunit_link = dispatcher->workunits;
		dispatcher->workunits = dispatcher->workunits->next;
		worker->workunit_link->next = NULL;
		send_workunit(worker, (t_workunit *)(worker->workunit_link->data));
	}
	pthread_mutex_unlock(&dispatcher->workunits_mutex);
//	diff = clock() - start;
//	msec = diff * 1000 / CLOCKS_PER_SEC;
//	printf("Handle_workunit_req mutex lock took %d seconds %d milliseconds\n", msec/1000, msec%1000);
	if (DEBUG && MUTEX_DEBUG)
		printf("*work units mutex unlocked!\n");
	free(msg.data);
}
