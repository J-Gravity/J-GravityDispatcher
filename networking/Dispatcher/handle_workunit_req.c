/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_workunit_req.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/25 16:35:24 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_workunit_req(t_dispatcher *dispatcher, t_worker *worker, t_msg msg)
{
	t_lst	*delete_me;

	//printf("A$\n");
	pthread_mutex_lock(&dispatcher->workunits_mutex);
	if (dispatcher->workunits)
	{
		worker->workunit_link = dispatcher->workunits;
		//printf("B$\n");
		dispatcher->workunits = dispatcher->workunits->next;
		//printf("C$\n");
		worker->workunit_link->next = NULL;
		//printf("D$\n");
		send_workunit(worker, (t_workunit *)(worker->workunit_link->data));
		//printf("E$\n");
	}
	// else
	// 	send_worker_msg(worker, new_message(NO_WORK_UNITS, 1, " "));
	pthread_mutex_unlock(&dispatcher->workunits_mutex);
	//printf("F$\n");
	free(msg.data);
	//printf("finished handle workunit\n");
}
