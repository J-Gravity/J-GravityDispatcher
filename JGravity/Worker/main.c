/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/03 21:59:51 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/23 14:39:35 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//#include <unistd.h>
#include "worker.h"

static void print_debug(int fd, t_msg msg)
{
	char *line;

	if (msg.id == WORK_UNIT_REQUEST)
		line = "WORK_UNIT_REQUEST";
	else if (msg.id == WORK_UNIT_DONE)
		line = "WORK_UNIT_DONE";
	else
		line = "UNKNOWN";
	printf("SENT %zu bytes '%s' TO dispatcher %d\n", msg.size, line, fd);
}

void	send_msg(int fd, t_msg msg)
{
	char	*buffer;
	size_t		msg_size;

	msg_size = HEADER_SIZE + msg.size;
	buffer = (char *)calloc(1, msg_size);
	buffer[0] = msg.id;
	memcpy(&buffer[1], &msg.size, sizeof(size_t));
	memcpy(&buffer[HEADER_SIZE], msg.data, msg.size);
	send(fd, buffer, msg_size, 0);
	if (DEBUG & MSG_DEBUG)
		print_debug(fd, msg);
	if (msg.data)
		free(msg.data);
	if (buffer)
		free(buffer);
}

static void unlink_semaphores(t_worker *worker)
{
	CloseHandle(worker->sender_thread_sem);
	CloseHandle(worker->calc_thread_sem);
	CloseHandle(worker->debundle_sem);
	CloseHandle(worker->exit_sem);
	CloseHandle(worker->ready_for_bundle);
}

static void initialize_semaphores(t_worker *worker)
{
	unlink_semaphores(worker);
	worker->calc_thread_sem = CreateSemaphore(NULL, 0, INFINITY, NULL);
	worker->sender_thread_sem = CreateSemaphore(NULL, 0, INFINITY, NULL);
	worker->exit_sem = CreateSemaphore(NULL, 0, INFINITY, NULL);
	worker->debundle_sem = CreateSemaphore(NULL, 0, INFINITY, NULL);
	worker->ready_for_bundle = CreateSemaphore(NULL, 1, INFINITY, NULL);
}

int main(int argc, char **argsv)
{
	int			err;
	int			conn_socket;
	struct		sockaddr_in serv_addr;
	t_worker	*worker;
    WORD wVersionRequested;
    WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
	G_softening = SOFTENING;
	G_time_step = TIME_STEP;
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

	initialize_semaphores(worker);
	if (DEBUG)
		printf("semaphores initalized\n");
	launch_event_thread(worker);
	launch_calculation_thread(worker);
	launch_sender_thread(worker);
	launch_debundle_thread(worker);
	if (DEBUG)
		printf("threads launched\n");
	WaitForSingleObject(worker->exit_sem, INFINITE);
	clock_t diff = clock() - G_start_time;
	double sum = G_total_event_time + G_total_calc_time + G_total_send_time;
	if (METRICS)
	{
		printf("Total runtime: %.2lu\n", time(NULL) - G_start_time);
		printf("%-24s %-3ld %-6.2f%%\n", "Time spent eventing:",
				G_total_event_time, (G_total_event_time / sum) * 100);
		printf("%-24s %-3ld %-6.2f%%\n", "Time spent calculating:",
				G_total_calc_time, (G_total_calc_time / sum) * 100);
		printf("%-24s %-3ld %-6.2f%%\n", "Time spent sending:",
				G_total_send_time, (G_total_send_time / sum) * 100);
	}
	printf("worker done, good bye\n");
	//cleanup
	unlink_semaphores(worker);
	return (0);
}
