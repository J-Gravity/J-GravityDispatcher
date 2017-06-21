/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configure_simulation.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/19 17:18:04 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/20 21:23:42 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

void configure_simulation(t_worker *worker, t_msg msg)
{
	size_t offset = 0;

	memcpy(&(time_step), msg.data + offset, sizeof(float));
	offset += sizeof(float);
	memcpy(&(softening), msg.data + offset, sizeof(float));
	offset += sizeof(float);
	if (offset != msg.size)
	{
		printf("Warning: settings message may have more settings"
			"than were read and applied\n");
	}
	if (DEBUG)
	{
		printf("softening factor: %f\n", softening);
		printf("timestep: %f\n", time_step);
	}
	send_msg(worker->socket.fd, (t_msg){SETTINGS_APPLIED, 1, strdup(" ")});
}