/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_worker_msg.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/13 21:57:36 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/29 14:54:03 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

static void print_debug(int fd, t_msg msg)
{
	char *line;

	if (msg.id == WORK_UNIT_DONE)
		line = "WORK_UNIT_DONE";
	else if (msg.id == WORK_UNIT_REQUEST)
		line = "WORK_UNIT_REQUEST";
	printf("RECIEVED '%s' FROM worker %d\n", line, fd);
}

static void	check_for_errors(int bytes_read, int *error)
{
	if (bytes_read == -1)
		*error = -1;
	else if (bytes_read == 0)
		*error = 0;
}

char	read_header(int fd, t_msg *msg)
{
	char	*header;
	int		bytes_read;

	header = (char *)calloc(1, HEADER_SIZE);
	bytes_read = recv(fd, header, HEADER_SIZE, 0);
	if (bytes_read == HEADER_SIZE)
	{
		msg->id = header[0];
		if (DEBUG && MSG_DEBUG)
			print_debug(fd, *msg);
		memcpy(&msg->size, &header[1], sizeof(int));
		msg->data = (char *)calloc(1, msg->size);
	}
	else
	{
		printf("recieved a bad msg header on fd %d!\n", fd);
		check_for_errors(bytes_read, &msg->error);
		free(header);
		return (1);
	}
	free(header);
	return (0);
}

char	read_body(int fd, t_msg *msg)
{
	int	bodybytes;
	int	recv_bytes;
	
	bodybytes = 0;
	while (bodybytes < msg->size)
	{
		recv_bytes = recv(fd, msg->data + bodybytes, msg->size, 0);
		if (recv_bytes > 0)
			bodybytes += recv_bytes;
		else
			break;
	}
	check_for_errors(recv_bytes, &msg->error);
	if (bodybytes != msg->size)
	{
		printf("msg body should be %d bytes, but is only %d bytes!\n",
			msg->size, bodybytes);
		return (1);
	}
	return (0);
}

t_msg	get_worker_msg(t_worker *worker)
{

	int 	recv_bytes;
	t_msg	msg;

	msg.error = 42;
	msg.id = 0;
	msg.size = 0;
	read_header(worker->socket.fd, &msg);
	read_body(worker->socket.fd, &msg);
	return (msg);
}
