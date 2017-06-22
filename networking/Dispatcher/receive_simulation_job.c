/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   receive_simulation_job.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/21 17:34:10 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/21 20:51:49 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void receive_simulation_job(t_dispatcher *dispatcher)
{
	int		fd;
	t_msg	msg;

	fd = accept(dispatcher->sin.fd, (struct sockaddr *)&(dispatcher->sin.addr.sin_addr), &(dispatcher->sin.addrlen));
	if (fd == 0)
		printf("website accept returned 0!");
	else if (fd == -1)
		printf("website accept call failed with %d\n", errno);
	else
	{
		msg = recieve_msg(fd);
		//deserialize and save parameters
	}
	
}