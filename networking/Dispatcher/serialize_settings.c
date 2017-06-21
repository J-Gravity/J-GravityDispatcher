/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serialize_settings.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/20 15:40:31 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/20 19:25:39 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_msg	serialize_settings(t_dispatcher *dispatcher)
{
	size_t	offset = 0;
	t_msg	msg;

	msg.id = WORKER_SETTINGS;
	msg.size = sizeof(float) * 2;
	msg.data = calloc(2, sizeof(float));
	memcpy(msg.data + offset, &(dispatcher->timestep), sizeof(float));
	offset += sizeof(float);
	memcpy(msg.data + offset, &(dispatcher->softening), sizeof(float));
	offset += sizeof(float);
	return(msg);
}