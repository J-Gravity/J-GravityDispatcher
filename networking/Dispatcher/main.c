/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:48:50 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/23 14:22:12 by cyildiri         ###   ########.fr       */
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
	char		*line = NULL;
	size_t		size;

	if (ac != 2)
	{
		printf("Usage ./a.out [File name]\n");
		return (0);
	}
	signal(SIGPIPE, SIG_IGN);
	if (METRICS)
		initialize_metrics();
	dispatcher = (t_dispatcher	*)calloc(1, sizeof(t_dispatcher));
	printf("dispatcher struct is %lu bytes!!/n", sizeof(t_dispatcher));
	dispatcher->sin = setup_server_socket(PORT);
	dispatcher->ticks_cnt = 2;
	dispatcher->name = "66jobfair";
	dispatcher->is_connect = 1;
	dispatcher->is_running = 0;
	dispatcher->timestep = TIME_STEP;
	dispatcher->softening = SOFTENING;
	dispatcher->bundles = (t_queue *)calloc(1, sizeof(t_queue));
	pthread_mutex_init(&dispatcher->bundles->mutex, NULL);
	dispatcher->workers_queue = (t_queue *)calloc(1, sizeof(t_queue));
	pthread_mutex_init(&dispatcher->workers_queue->mutex, NULL);
	int ret = pthread_mutex_init(&dispatcher->workunits_mutex, NULL);
	pthread_mutex_init(&dispatcher->workunits_done_mutex, NULL);
	pthread_mutex_init(&dispatcher->worker_list_mutex, NULL);
	pthread_mutex_init(&dispatcher->output_mutex, NULL);
	pthread_mutex_init(&dispatcher->sender_thread_mutex, NULL);
	if (ret)
		printf("mutex init failed!!!!!!!!!!!\n");
	clock_t start = clock(), diff;
	receive_simulation_job(dispatcher);
	connect_workers(dispatcher, NULL);
	diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	if (METRICS && STARTUP_METRICS)
		printf("connect_workers took %d seconds %d milliseconds\n", msec/1000, msec%1000);
	start = clock();
	request_dataset(dispatcher, av[1]);
	diff = clock() - start;
	msec = diff * 1000 / CLOCKS_PER_SEC;
	if (METRICS && STARTUP_METRICS)
		printf("request_dataset took %d seconds %d milliseconds\n", msec/1000, msec%1000);
	launch_simulation(dispatcher); // blocks thread until all workers are done.
	return (0);
}
