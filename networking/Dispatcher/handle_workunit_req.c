/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_workunit_req.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/28 19:29:26 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_workunit_req(t_dispatcher *dispatcher, t_worker *worker, t_msg msg)
{
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
	// else
	// 	send_worker_msg(worker, new_message(NO_WORK_UNITS, 1, " "));
	pthread_mutex_unlock(&dispatcher->workunits_mutex);
	if (DEBUG && MUTEX_DEBUG)
		printf("*work units mutex unlocked!\n");
	free(msg.data);
}
