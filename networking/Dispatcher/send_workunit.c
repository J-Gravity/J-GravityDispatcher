/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_workunit.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:42:08 by scollet           #+#    #+#             */
/*   Updated: 2017/05/25 19:39:04 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int		send_workunit(t_worker *worker, t_workunit *work_unit)
{
	t_msg	msg;

	//printf("START send_work_unit\n");
	msg = serialize_workunit(*work_unit);
	msg.id = WORK_UNIT;
	worker->w_calc_time = time(NULL);
	send_worker_msg(worker, msg);
	free(msg.data);
	//printf("f\n");
	return (0);
}
