/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configure_worker_settings.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/29 00:12:34 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/29 01:35:28 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	configure_worker_settings(t_dispatcher *dispatcher, t_worker *worker)
{
	t_msg	settings_msg;
	t_msg	response;

	settings_msg = serialize_settings(dispatcher);
	send_worker_msg(worker, settings_msg);
	free(settings_msg.data);

	response = receive_msg(worker->socket.fd);
	if (response.id != SETTINGS_APPLIED)
		printf("ERROR: Worker Failed to respond to SETTINGS MSG\n");
	else
		printf("Worker settings applied successfully!!\n");
}
