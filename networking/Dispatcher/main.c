/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:48:50 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/29 01:49:46 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void initialize_metrics()
{
	G_tick_start = 0;
	G_workunit_size = 0;
	G_total_workunit_size = 0;
	G_worker_calcs = 0;
	G_total_time = 0;
	G_total_workunit_cnt = 0;
}

int	main(int ac, char **av)
{
	t_dispatcher	*dispatcher;
	size_t			size;

	TIMER_SETUP(msec)
	if (ac != 2)
	{
		printf("Usage ./a.out [File name]\n");
		return (0);
	}
	signal(SIGPIPE, SIG_IGN);
	if (METRICS)
		initialize_metrics();
	dispatcher = new_dispatcher(PORT, 4224);
	dispatcher->name = "default";
	dispatcher->ticks_cnt = 10;
	dispatcher->timestep = TIME_STEP;
	dispatcher->softening = SOFTENING;
//	receive_simulation_job(dispatcher);
	
	TIMER_START(msec)
	connect_workers(dispatcher, NULL);
	TIMER_STOP(msec)
	TIMER_PRINT("connect_workers", msec)

	TIMER_START(msec)
	load_dataset(dispatcher, av[1]);
	TIMER_STOP(msec)
	TIMER_PRINT("load_dataset", msec)

	launch_simulation(dispatcher); // blocks thread until all workers are done.
	return (0);
}
