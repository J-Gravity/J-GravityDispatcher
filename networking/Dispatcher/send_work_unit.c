/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_work_unit.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:42:08 by scollet           #+#    #+#             */
/*   Updated: 2017/05/14 21:55:09 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int   send_work_unit(t_worker *worker, t_work_unit work_unit)
{
	t_msg	msg;

	msg = serialize_work_unit(work_unit);
	send_worker_msg(worker, 1, msg->size, msg->data);
	return (0);
}
