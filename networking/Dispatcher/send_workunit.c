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

	//printf("A]\n");
	//print_cl4(work_unit->local_bodies[0].position);
	//print_cl4(work_unit->local_bodies[0].velocity);
	msg = serialize_workunit2(*work_unit);
	//printf("B]\n");
	msg.id = WORK_UNIT;
	//printf("size being sent: %d\n", msg.size + 5);
	//printf("C]\n");
	send_worker_msg(worker, msg);
	//printf("D]\n");
	free(msg.data);
	//printf("E]\n");
	return (0);
}
