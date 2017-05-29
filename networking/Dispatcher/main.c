/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:48:50 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/29 14:46:27 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int	main(int ac, char **av)
{
	t_dispatcher	*dispatcher;

	if (ac != 2)
	{
		printf("Usage ./a.out [File name]\n");
		return (0);
	}
	signal(SIGPIPE, SIG_IGN);
	dispatcher = (t_dispatcher	*)calloc(1, sizeof(t_dispatcher));
	dispatcher->sin = setup_server_socket(PORT);
	dispatcher->ticks_cnt = 4;
	dispatcher->name = "mvp_test";
	dispatcher->is_connect = 1;
	dispatcher->is_running = 0;
	int ret = pthread_mutex_init(&dispatcher->workunits_mutex, NULL);
	pthread_mutex_init(&dispatcher->workunits_done_mutex, NULL);
	pthread_mutex_init(&dispatcher->worker_list_mutex, NULL);
	if (ret)
		printf("mutex init failed!!!!!!!!!!!\n");
	connect_workers(dispatcher, &dispatcher->workers);
	request_dataset(dispatcher, av[1]);
	divide_dataset(dispatcher);
	launch_simulation(dispatcher); // blocks thread until all workers are done.
	return (0);
}
