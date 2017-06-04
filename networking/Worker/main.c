/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/03 21:59:51 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/03 17:28:00 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include <dispatch/dispatch.h>
#include "worker.h"

void	send_msg(int fd, t_msg msg)
{
	char	*buffer;
	int		msg_size;

	msg_size = 5 + msg.size;
	buffer = (char *)calloc(1, msg_size);
	buffer[0] = msg.id;
	memcpy(&buffer[1], &msg.size, sizeof(int));
	memcpy(&buffer[5], msg.data, msg.size);
	send(fd, buffer, msg_size, 0);
	if (msg.data)
		free(msg.data);
	if (buffer)
		free(buffer);
}

int main(int argc, char **argsv)
{
	int			err;
	int			conn_socket;
	struct		sockaddr_in serv_addr;
	t_worker	*worker;

	worker = (t_worker *)calloc(1, sizeof(t_worker));
	if (argc == 1)
	{
		printf("Usage ./a.out [IP Address]\n");
		exit(1);
	}
	conn_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn_socket == -1)
		write(1, "sock error occured\n", 19);

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(4242);
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, argsv[1], &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}
	if (connect(conn_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	printf("Successfully connected to %s\n", argsv[1]);
	worker->socket.fd = conn_socket;
	worker->todo_work = (t_queue *)calloc(1, sizeof(t_queue));
	worker->todo_work->last = (t_lst *)calloc(1, sizeof(t_lst));
	worker->completed_work = (t_queue *)calloc(1, sizeof(t_queue));
	launch_event_thread(worker);
	launch_calculation_thread(worker);
	launch_sender_thread(worker);
	sleep(99999999);
	//cleanup
	return (0);
}
