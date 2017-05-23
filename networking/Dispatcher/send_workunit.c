/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_workunit.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:42:08 by scollet           #+#    #+#             */
/*   Updated: 2017/05/22 18:15:18 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int		send_workunit(t_worker *worker, t_workunit *work_unit)
{
	t_msg	msg;

	printf("A]\n");
	msg = serialize_workunit(work_unit);
	printf("B]\n");
	msg.id = 1;
	printf("C]\n");
	send_worker_msg(worker, msg);
	printf("D]\n");
	return (0);
}
