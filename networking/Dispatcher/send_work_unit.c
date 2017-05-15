/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_work_unit.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:42:08 by scollet           #+#    #+#             */
/*   Updated: 2017/05/14 22:14:31 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int   send_work_unit(t_worker *worker, t_work_unit work_unit)
{
	t_msg	msg;

	msg = serialize_work_unit(work_unit);
	msg.id = 1;
	send_worker_msg(worker, msg);
	return (0);
}
