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
	printf("SENT '%s' TO worker %d\n", line, worker->socket.fd);
}

void	send_worker_msg(t_worker *worker, t_msg msg)
{
	char	*buffer;
	int		msg_size;

	if (worker->socket.fd == 0)
		return ;
	//printf("A*\n");
	msg_size = HEADER_SIZE + msg.size;
	//printf("B*\n");
	buffer = (char *)calloc(1, msg_size);
	//printf("C*\n");
	buffer[0] = msg.id;
	//printf("D*\n");
	memcpy(&buffer[1], &msg.size, sizeof(int));

	//printf("msg_size %d\n", msg_size);

	//printf("E*\n");
	memcpy(&buffer[5], msg.data, msg.size);
	//printf("F*\n");
	send(worker->socket.fd, buffer, msg_size, 0);
	print_debug(worker, msg);
	//printf("G*\n");
	//free(msg.data);
	//printf("H*\n");

	free(buffer);
}
