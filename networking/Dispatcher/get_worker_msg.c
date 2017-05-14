/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_worker_msg.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/13 21:57:36 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/13 21:57:37 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <dispatcher.h>

t_msg	get_worker_msg(t_worker *worker)
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
		msg.data = (char *)calloc(1, msg.size);
		bytes_read = recv(worker->socket.fd, msg.data, msg.size, 0);
		if (bytes_read != msg.size)
			printf("msg size should be %d bytes,"
			"but is only %d bytes!\n", msg.size, bytes_read);
		check_for_errors(bytes_read, &msg.error);
	}
	check_for_errors(bytes_read, &msg.error);
	return (msg);
}
