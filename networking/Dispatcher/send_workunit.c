/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_workunit.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:42:08 by scollet           #+#    #+#             */
/*   Updated: 2017/06/14 00:13:37 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int		send_bundle(t_worker *worker, t_bundle *bundle, t_tree **leaves)
{
	t_msg	msg;
	msg = serialize_bundle(bundle, leaves);
	msg.id = WORK_UNIT;
	worker->w_calc_time = time(NULL);
	if (worker->active == 0)
	  printf("sending a bundle to an inactive worker\n");
	send_worker_msg(worker, msg);
	free(msg.data);
	return (0);
}

int		send_workunit(t_worker *worker, t_workunit *work_unit)
{
	t_msg	msg;
	if (DEBUG)
	  printf("sending wu: %d", work_unit->id);
	msg = serialize_workunit(*work_unit);
	msg.id = WORK_UNIT;
	worker->w_calc_time = time(NULL);
	send_worker_msg(worker, msg);
	free(msg.data);
	return (0);
}
