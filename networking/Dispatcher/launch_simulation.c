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

#define HEADER_SIZE 5
t_msg	get_network_msg()
{

}

static void 	handle_worker_connection(t_lst	*worker)
{
	char		*buffer;
	int			bytes_read;
	t_worker	*cur_worker;

	cur_worker = (t_worker *)worker->data;
	if (worker->next)
	{
		pthread_create(	cur_worker->event_tid, /*SOMETHING*/,
						handle_worker_connection, worker->next);
	}
	while (cur_worker->socket.fd)
	{
		//wait for network msg
		bytes_read = recv(cur_worker->socket.fd, buffer, HEADER_SIZE, 0);
		if (bytes_read == HEADER_SIZE)
		{

		}
		else if (bytes_read == 0)
		{
			//attempt to reconnect to same worker
		}
		else
		{
			//bad network message
		}
	}

}


void			launch_simulation(t_dispatcher *dispatcher)
{
	//launch worker network event threads

}
