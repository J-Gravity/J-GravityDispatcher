/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_worker_msg.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/13 21:57:36 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/15 00:39:15 by cyildiri         ###   ########.fr       */
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

static void	check_for_errors(ssize_t bytes_read, int *error)
{
	if (bytes_read == -1)
		*error = -1;
	else if (bytes_read == 0)
		*error = 0;
}

char	read_header(int fd, t_msg *msg)
{
	char	*header;
	ssize_t	bytes_read;

	header = (char *)calloc(1, HEADER_SIZE);
	bytes_read = recv(fd, header, HEADER_SIZE, 0);
	if (bytes_read == HEADER_SIZE)
	{
		msg->id = header[0];
 		if (DEBUG && MSG_DEBUG)
			print_debug(fd, *msg);
		memcpy(&msg->size, &header[1], sizeof(size_t));
		msg->data = (char *)calloc(1, msg->size + 1);
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
	size_t	bodybytes;
	ssize_t	recv_bytes;
	
	bodybytes = 0;
	while (bodybytes < msg->size)
	{
		recv_bytes = recv(fd, msg->data + bodybytes, msg->size - bodybytes, 0);
		if (recv_bytes > 0)
			bodybytes += recv_bytes;
		else
			break;
	}
	check_for_errors(recv_bytes, &msg->error);
	if (bodybytes != msg->size)
	{
		printf("msg body should be %zu bytes, but is only %zu bytes!\n",
			msg->size, bodybytes);
		return (1);
	}
	return (0);
}

t_msg	get_msg(int fd)
{

	int 	recv_bytes;
	t_msg	msg;

	msg.error = 42;
	msg.id = 0;
	msg.size = 0;
	read_header(fd, &msg);
	read_body(fd, &msg);
	return (msg);
}
