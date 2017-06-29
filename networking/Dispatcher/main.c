/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:48:50 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/28 22:23:28 by cyildiri         ###   ########.fr       */
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

t_queue			*new_queue()
{
	t_queue *queue;

	queue = (t_queue *)calloc(1, sizeof(t_queue));
	pthread_mutex_init(&queue->mutex, NULL);
	return (queue);
}

t_dispatcher	*new_dispatcher()
{
	t_dispatcher	*dispatcher;

	dispatcher = (t_dispatcher	*)calloc(1, sizeof(t_dispatcher));
	dispatcher->sin = setup_server_socket(PORT);
	dispatcher->cmd_sin = setup_server_socket(4224);
	dispatcher->bundles = new_queue();
	dispatcher->workers_queue = new_queue();
	dispatcher->is_connect = 1;
	dispatcher->is_running = 0;
	pthread_mutex_init(&dispatcher->workunits_mutex, NULL);
	pthread_mutex_init(&dispatcher->workunits_done_mutex, NULL);
	pthread_mutex_init(&dispatcher->worker_list_mutex, NULL);
	pthread_mutex_init(&dispatcher->output_mutex, NULL);
	pthread_mutex_init(&dispatcher->sender_thread_mutex, NULL);
	return (dispatcher);
}

int	main(int ac, char **av)
{
	t_dispatcher	*dispatcher;
	char		*line = NULL;
	size_t		size;

	TIMER_SETUP(msec)
	if (ac != 2)
	{
		printf("Usage ./a.out [File name]\n");
		return (0);
	}
	signal(SIGPIPE, SIG_IGN);
	if (METRICS)
		initialize_metrics();
	dispatcher = new_dispatcher();
	dispatcher->name = "default";
	dispatcher->ticks_cnt = 10;
	dispatcher->timestep = TIME_STEP;
	dispatcher->softening = SOFTENING;
	receive_simulation_job(dispatcher);
	
	TIMER_START(msec)
	connect_workers(dispatcher, NULL);
	TIMER_STOP(msec)
	TIMER_PRINT("connect_workers", msec)

	TIMER_START(msec)
	request_dataset(dispatcher, av[1]);
	TIMER_STOP(msec)
	TIMER_PRINT("load_dataset", msec)

	launch_simulation(dispatcher); // blocks thread until all workers are done.
	return (0);
}
