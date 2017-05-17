/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_worker_done_msg.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 16:35:38 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/17 00:09:41 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_worker_done_msg(t_dispatcher *dispatcher, t_worker *worker,
		t_msg msg)
{
	t_work_unit	new_work_unit;
	t_work_unit	*old_work_unit;
	int			i;

	new_work_unit = deserialize_work_unit(msg);
	old_work_unit = worker->work_unit;
	i = 0;
	while (i < old_work_unit->cell.body_count)
	{
		memcpy(old_work_unit->cell.contained_bodies[i], new_work_unit.cell.contained_bodies[i], sizeof(t_body));
		i++;
	}
	old_work_unit->complete = 1;
	send_worker_msg(worker, new_message(ACKNOWLEDGED, 0, ""));
}
