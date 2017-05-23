/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_workunit_req.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/23 14:08:26 by cyildiri         ###   ########.fr       */
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
		//printf("B$\n");
		send_workunit(worker, (t_workunit *)(dispatcher->workunits->data));
		//printf("C$\n");
		delete_me = dispatcher->workunits;
		//printf("D$\n");
		dispatcher->workunits = dispatcher->workunits->next;
		//printf("E$\n");
		clear_unit(&delete_me);
	}
	// else
	// 	send_worker_msg(worker, new_message(NO_WORK_UNITS, 1, " "));
	pthread_mutex_unlock(&dispatcher->workunits_mutex);
	//printf("F$\n");
}
