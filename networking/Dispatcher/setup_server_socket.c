/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_server_socket.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 19:52:54 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/10 14:42:40 by elee             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_socket		setup_server_socket(int port)
{
	t_socket	sin;
	int optval;
 
	memset(&sin.addr, 0, sizeof(sin.addr));
	sin.addrlen = sizeof(struct sockaddr_storage);
	//sin.addr.sin_len = sizeof(sin.addr);
	sin.addr.sin_family = AF_INET; /* Address family */
	sin.addr.sin_port = htons(port); /* Or a specific port */
	sin.addr.sin_addr.s_addr= INADDR_ANY;
	if ((sin.fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		printf("Creating the server socket failed with code: %d\n", errno);
		exit(1);
	}
	/*
	sin.addr.sin_family = AF_INET;
	sin.addr.sin_addr.s_addr = INADDR_ANY;
	sin.addr.sin_port = htons(port);
	*/

	optval = 1;
	setsockopt(sin.fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	if (bind(sin.fd, (struct sockaddr *)&sin.addr, sizeof(sin.addr)) < 0)
	{
		fprintf(stderr, "Binding the server socket failed with code: %d\n", errno);
		exit(1);
	}
	if (listen(sin.fd, 3) < 0)
	{
		printf("Listening on server socket failed with code: %d\n", errno);
		exit(1);
	}
	printf("sin.fd == %d\n", sin.fd);
	return (sin);
}
