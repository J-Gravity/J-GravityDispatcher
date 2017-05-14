/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_worker_msg.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/13 21:38:19 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/13 21:38:21 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	send_worker_msg(t_worker *worker, char id, int data_size, char *data)
{
	char	*buffer;
	int		msg_size;

	msg_size = HEADER_SIZE + data_size;
	buffer = (char *)calloc(1, msg_size);
	buffer[0] = id;
	memcpy(&buffer[1], data, data_size);
	send(worker->socket.fd, buffer, msg_size, 0);
	free(buffer);
}
