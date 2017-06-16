/* ************************************************************************** */

/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_worker_msg.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/13 21:38:19 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/23 19:24:46 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

static void print_debug(t_worker *worker, t_msg msg)
{
	char *line;

	if (msg.id == WORK_UNITS_READY)
		line = "WORK_UNITS_READY";
	else if (msg.id == WORK_UNIT)
		line = "WORK_UNIT";
	printf("SENT %zu byte '%s' TO worker %d\n", msg.size, line, worker->socket.fd);
}

void	send_worker_msg(t_worker *worker, t_msg msg)
{
	char	*buffer;
	size_t		msg_size;

	if (worker->socket.fd == 0)
		return ;
	msg_size = HEADER_SIZE + msg.size;
	buffer = (char *)calloc(1, msg_size);
	buffer[0] = msg.id;
	memcpy(&buffer[1], &msg.size, sizeof(size_t));
	memcpy(&buffer[HEADER_SIZE], msg.data, msg.size);
	if (send(worker->socket.fd, buffer, msg_size, 0) == -1)
	{
		printf("send failed with %d\n", errno);
		worker->active = 0;
	}
	if (DEBUG && MSG_DEBUG)
		print_debug(worker, msg);
	free(buffer);
}
