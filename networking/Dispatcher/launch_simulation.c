/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/11 20:53:10 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

static void	check_for_errors(int bytes_read, int *error)
{
	if (bytes_read == -1)
		*error = -1;
	else if (bytes_read == 0)
		*error = 0;
}

t_msg		get_worker_msg(t_worker	*worker)
{
	char	*buffer;
	int		bytes_read;
	t_msg	msg;

	msg.error = 42;
	buffer = (char *)calloc(1, HEADER_SIZE);
	bytes_read = recv(worker->socket.fd, buffer, HEADER_SIZE, 0);
	if (bytes_read == HEADER_SIZE)
	{
		msg.id = buffer[0];
		memcpy(&msg.size, &buffer[1], 4);
		msg.data = (char *)calloc(msg.size);
		bytes_read = recv(worker->socket.fd, msg.data, msg.size, 0);
		if (bytes_read != msg.size)
			printf("msg size should be %d bytes,"
			"but is only %d bytes!\n", msg.size, bytes_read);
		check_for_errors(bytes_read, &msg.error);
	}
	check_for_errors(bytes_read, &msg.error);
	return (msg)
}

static void	handle_worker_msg(t_msg msg)
{
	if (msg.id == A)
	else if (msg.id == B)
	else if (msg.id == C)
	else if (msg.id == D)
	else if (msg.id == E)
	else if (msg.id == F)
	else
		printf("invalid msg id: %d", msg.id);
}

static void	handle_worker_connection(t_lst	*worker)
{
	t_msg		msg;
	t_worker	*cur_worker;
	t_worker	*next_worker;


	cur_worker = (t_worker *)worker->data;
	//signal worker that work is available
	//send(cur_worker->socket.fd, );

	while (1)
	{
		if (worker->next && ((t_worker *)worker->next)->tid == 0)
		{
			next_worker = (t_worker *)worker->next->data;
			pthread_create(	next_worker->event_tid, NULL,
							handle_worker_connection, worker->next);
		}
		//wait for network msg on this worker's conneciton
		msg = get_worker_msg(cur_worker);
		if (msg.error == -1)
			printf("get worker message failed with err %d\n", errno);
		else
			handle_worker_msg(msg);
	}
}

void		launch_simulation(t_dispatcher *dispatcher)
{
	t_worker	*cur_worker;

	cur_worker = (t_worker *)dispatcher->workers->data;
	//launch worker network event threads
	pthread_create(	cur_worker->event_tid, NULL,
					handle_worker_connection, dispatcher->workers);
}
