/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_workunit.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:42:08 by scollet           #+#    #+#             */
/*   Updated: 2017/05/23 02:01:14 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int		send_workunit(t_worker *worker, t_workunit *work_unit)
{
	t_msg	msg;

	msg = serialize_workunit(*work_unit);
	msg.id = WORK_UNIT;
	send_worker_msg(worker, msg);
	free(msg.data);
	return (0);
}
