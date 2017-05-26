/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_worker_msg.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/13 21:57:36 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/25 19:36:22 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

static void print_debug(t_worker *worker, t_msg msg)
{
	char *line;

	if (msg.id == WORK_UNIT_DONE)
		line = "WORK_UNIT_DONE";
	else if (msg.id == WORK_UNIT_REQUEST)
		line = "WORK_UNIT_REQUEST";
	//printf("RECIEVED '%s' FROM worker %d\n", line, worker->socket.fd);
}

static void	check_for_errors(int bytes_read, int *error)
{
	if (bytes_read == -1)
		*error = -1;
	else if (bytes_read == 0)
		*error = 0;
}

t_msg	get_worker_msg(t_worker *worker)
{
	char	*buffer;
	int		bytes_read;
	t_msg	msg;

	printf("START get_worker_msg\n");
	msg.error = 42;
	msg.id = 0;
	msg.size = 0;
	msg.data = calloc(1, 1);
	buffer = (char *)calloc(1, HEADER_SIZE);
	bytes_read = recv(worker->socket.fd, buffer, HEADER_SIZE, 0);
	if (bytes_read == HEADER_SIZE)
	{
		msg.id = buffer[0];
		print_debug(worker, msg);
		memcpy(&msg.size, &buffer[1], sizeof(int));
		msg.data = (char *)calloc(1, msg.size);
		int bodybytes = 0;
		while (bodybytes < msg.size)
			bodybytes += recv(worker->socket.fd, msg.data + bodybytes, msg.size, 0);
		if (bodybytes != msg.size)
			printf("msg size should be %d bytes, but is only %d bytes!\n", msg.size, bytes_read);
		check_for_errors(bytes_read, &msg.error);
	}
	check_for_errors(bytes_read, &msg.error);
	free(buffer);
	printf("gwm10\n");
	return (msg);
}
