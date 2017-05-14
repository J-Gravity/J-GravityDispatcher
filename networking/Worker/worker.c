/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   worker.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 16:08:11 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/14 11:37:22 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Dispatcher/dispatcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 4000

int		btoi(char *str)
{
	int				ret;

	for (unsigned int i = 0; i < sizeof(int); i++)
		((char *)(&ret))[i] = str[i];
	return (ret);
}

float	btof(char *str)
{
	float	ret;

	for (unsigned int i = 0; i < sizeof(float); i++)
		((char *)(&ret))[i] = str[i];
	return (ret);
}

t_work_unit	*work_unit_parser(char *str)
{
	int			i;
	t_work_unit	*work_unit;

	i = 0;
	work_unit = malloc(sizeof(t_work_unit));
	work_unit->compute_class = btoi(str += 4);
	work_unit->cell.body_count = btoi(str += 4);
	work_unit->cell.contained_bodies = (t_body **)malloc(sizeof(t_body *) * work_unit->cell.body_count);
	for (int i = 0; i < work_unit->cell.body_count; i++)
	{
		work_unit->cell.contained_bodies[i] = (t_body *)malloc(sizeof(t_body));
		work_unit->cell.contained_bodies[i]->position.x = btoi(str += 4);
		work_unit->cell.contained_bodies[i]->position.y = btoi(str += 4);
		work_unit->cell.contained_bodies[i]->position.z = btoi(str += 4);
		work_unit->cell.contained_bodies[i]->velocity.x = btoi(str += 4);
		work_unit->cell.contained_bodies[i]->velocity.y = btoi(str += 4);
		work_unit->cell.contained_bodies[i]->velocity.z = btoi(str += 4);
		work_unit->cell.contained_bodies[i]->mass = btoi(str += 4);
	}
	return (work_unit);
}

void	serializer_identifier(char *str)
{
	printf("\nserializer id: %d\n", btoi(str));
	if (btoi(str) == 1)
	{
		printf("yes\n");
		work_unit_parser(str);
	}
}

void	looper(struct sockaddr_in client, int server_fd, int client_fd)
{
	int			err;
	int			read;
	char		buf[BUFFER_SIZE];
	socklen_t	client_len;

	while (1)
	{
		client_len = sizeof(client);
		client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);
		if (client_fd < 0)
			printf("Could not establish new connection\n");
		while (1)
		{
			read = recv(client_fd, buf, BUFFER_SIZE, 0);
			printf("%s", buf);
			if (!read)
				break ;
			if (read < 0)
				printf("Client read failed\n");
			err = send(client_fd, buf, read, 0);
			if (err < 0)
				printf("Client write failed\n");
			bzero(buf, strlen(buf));
		}
	}
}

#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MYPORT "3490"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

int main(void)
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
        printf("Could not create socket");
    puts("Socket created");
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
    puts("Connected\n");
    //keep communicating with server
    while(1)
    {
        message = strdup("ready");
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }
        work_unit_parser(server_reply);
        puts(server_reply);
    }
    close(sock);
    return 0;
}
