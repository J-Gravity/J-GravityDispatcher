/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/03 21:59:51 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/04 21:24:17 by ssmith           ###   ########.fr       */
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

static void print_debug(int fd, t_msg msg)
{
	char *line;

	if (msg.id == WORK_UNIT_REQUEST)
		line = "WORK_UNIT_REQUEST";
	else if (msg.id == WORK_UNIT_DONE)
		line = "WORK_UNIT_DONE";
	printf("SENT '%s' TO dispatcher %d\n", line, fd);
}

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
	if (DEBUG & MSG_DEBUG)
		print_debug(fd, msg);
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
	serv_addr.sin_port = htons(PORT);
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

	worker->calc_thread_sem = sem_open("/calc_thread", O_CREAT, 0644, 0);
	if (worker->calc_thread_sem == SEM_FAILED)
		printf("sem1 open failed with %d\n", errno);
	worker->sender_thread_sem = sem_open("/sender_thread", O_CREAT, 0644, 0);
	if (worker->sender_thread_sem == SEM_FAILED)
		printf("sem2 open failed with %d\n", errno);
	worker->exit_sem = sem_open("/exit", O_CREAT, 0644, 0);
	if (worker->exit_sem == SEM_FAILED)
		printf("sem3 open failed with %d\n", errno);
	if (DEBUG)
		printf("semaphores initalized\n");
	launch_event_thread(worker);
	launch_calculation_thread(worker);
	launch_sender_thread(worker);
	if (DEBUG)
		printf("threads launched\n");
	sleep(90000);
	int val = sem_wait(worker->exit_sem);
	if (val < 0)
		printf("wem_wait failed with err:%d\n", errno);
	printf("A exit sem: %d\n", val);
	//cleanup	
	printf("A\n");
	sem_close(worker->calc_thread_sem);
	printf("B\n");
	sem_close(worker->sender_thread_sem);
	printf("C\n");
	sem_close(worker->exit_sem);
	printf("D\n");
	
	int rc = sem_unlink("/calc_thread");
    if (rc)
        printf("sem_unlink err %d\n", errno);
	rc = sem_unlink("/sender_thread");
    if (rc)
        printf("sem_unlink err %d\n", errno);
	rc = sem_unlink("/exit");
    if (rc)
        printf("sem_unlink err %d\n", errno);
	printf("F\n");
	return (0);
}
