/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:48:50 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/22 13:16:34 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int	main(void)
{
	t_dispatcher	*dispatcher;

	dispatcher = (t_dispatcher	*)calloc(1, sizeof(t_dispatcher));
	dispatcher->sin = setup_server_socket(PORT);
	dispatcher->ticks_cnt = 18000;
	dispatcher->name = "mvp_test";
	dispatcher->is_connect = 1;
	dispatcher->is_running = 0;
	connect_workers(dispatcher, &dispatcher->workers);
	request_dataset(&dispatcher->dataset);
	divide_dataset(dispatcher);
	launch_simulation(dispatcher); // blocks thread until all workers are done.
	//dump_all_workers_cache(dispatcher);
	//coalesce_into_ticks(dispatcher);
	//save_output(dispatcher, "mvp_test");
	return (0);
}
