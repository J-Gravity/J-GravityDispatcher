/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configure_simulation.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/19 17:18:04 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/23 14:44:09 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

void configure_simulation(t_worker *worker, t_msg msg)
{
	size_t offset = 0;

	memcpy(&(G_time_step), msg.data + offset, sizeof(float));
	offset += sizeof(float);
	memcpy(&(G_softening), msg.data + offset, sizeof(float));
	offset += sizeof(float);
	if (offset != msg.size)
	{
		printf("Warning: settings message may have more settings"
			"than were read and applied\n");
	}
	if (DEBUG)
	{
		printf("softening factor: %f\n", G_softening);
		printf("timestep: %f\n", G_time_step);
	}
	send_msg(worker->socket.fd, (t_msg){SETTINGS_APPLIED, 1, strdup(" ")});
}