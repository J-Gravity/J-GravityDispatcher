/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_dispatcher.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/28 22:48:07 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/28 23:59:22 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_dispatcher	*new_dispatcher(int worker_port, int cmd_port)
{
	t_dispatcher	*dispatcher;

	dispatcher = (t_dispatcher	*)calloc(1, sizeof(t_dispatcher));
	dispatcher->sin = setup_server_socket(worker_port);
	dispatcher->cmd_sin = setup_server_socket(cmd_port);
	dispatcher->bundles = new_queue();
	dispatcher->workers = new_queue();
	dispatcher->is_connect = 1;
	dispatcher->is_running = 0;
	pthread_mutex_init(&dispatcher->bundles_done_mutex, NULL);
	pthread_mutex_init(&dispatcher->output_mutex, NULL);
	pthread_mutex_init(&dispatcher->sender_thread_mutex, NULL);
	return (dispatcher);
}
