/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/03 21:59:51 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/22 22:29:46 by cyildiri         ###   ########.fr       */
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

#include "worker.h"

t_msg	new_message(char id, int data_size, char *data)
{
	t_msg	message;

	message.id = id;
	message.size = data_size;
	message.data = (char *)calloc(1, data_size);
	memcpy(&message.data, data, data_size);
	return (message);
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
	printf("fd %d, size %d\n", fd, msg_size);
	send(fd, buffer, msg_size, 0);
	printf("finished send\n");
	free(msg.data);
	free(buffer);
}

t_msg	wait_for_msg(int socket, int message_code)
{
	char	*buffer;
	int		bytes_read;
	t_msg	msg;


	buffer = (char *)calloc(1, HEADER_SIZE);
	bytes_read = recv(socket, buffer, HEADER_SIZE, 0);
	printf("bytes read: %d\n", bytes_read);
	if (bytes_read == HEADER_SIZE)
	{
		int bodybytes = 0;
		msg.id = buffer[0];
		memcpy(&msg.size, &buffer[1], sizeof(int));
		msg.data = (char *)calloc(1, msg.size);
		while (bodybytes < msg.size)
		{
			bodybytes += recv(socket, msg.data + bodybytes, msg.size, 0);
		}
		printf("body bytes read %d\n", bodybytes);
		printf("total bytes read %d\n", bodybytes + bytes_read);
		//check_for_errors(bytes_read, &msg.error);

	}
	else
	{
		printf("something was wrong with the message\n");
		exit(1);
	}
	if (msg.id != message_code)
	{
		printf("not the message we were expecting\n");
		exit(1);
	}
	else
		return msg;
}

int main(int argc, char **argsv)
{
	int err;
	int conn_socket;
	struct sockaddr_in serv_addr;
	char	*header;
	int		buff_size;
	char 	*buffer;

	conn_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn_socket == -1)
		write(1, "sock error occured\n", 19);

	memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(4242);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argsv[1], &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }	

	if (connect(conn_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1)
    {
    	t_msg msg;
    	msg = wait_for_msg(conn_socket, WORK_UNITS_READY);
    	printf("got WU_READY\n");
    	send_msg(conn_socket, (t_msg){WORK_UNIT_REQUEST, 1, strdup(" ")});
    	printf("sent WU_REQ\n");
    	msg = wait_for_msg(conn_socket, WORK_UNIT);
    	printf("got WU\n");

    	t_workunit w = deserialize_workunit2(msg);
    	printf("workunit deserialized\n");
    	w = do_workunit(w);
    	printf("did workunit\n");
    	msg = serialize_workunit2(w);
    	printf("serialized\n");

//     	typedef struct			s_msg
// {
// 	char				id;
// 	int					size;
// 	char				*data;
// 	int					error;
// }						t_msg;

    	printf("msg.id %d, msg.size %d\n", msg.id, msg.size);
    	send_msg(conn_socket, msg);
    	printf("sent completed unit\n");
    }

	return (0);
}
