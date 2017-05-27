/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_workunit_req.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/27 15:55:33 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_workunit_req(t_dispatcher *dispatcher, t_worker *worker, t_msg msg)
{
	t_lst	*delete_me;

	pthread_mutex_lock(&dispatcher->workunits_mutex);
	if (dispatcher->workunits)
	{
		worker->workunit_link = dispatcher->workunits;
		dispatcher->workunits = dispatcher->workunits->next;
		worker->workunit_link->next = NULL;
		send_workunit(worker, (t_workunit *)(worker->workunit_link->data));
	}
	pthread_mutex_unlock(&dispatcher->workunits_mutex);
	free(msg.data);
}
