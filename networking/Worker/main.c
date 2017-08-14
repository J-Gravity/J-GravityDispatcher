/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/03 21:59:51 by cyildiri          #+#    #+#             */
/*   Updated: 2017/08/10 15:31:14 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "worker.h"

static void unlink_semaphores()
{
	int ret;

    ret = sem_unlink("/next_tick");
    if (DEBUG && ret)
        printf("sem_unlink err %d\n", errno);
	ret = sem_unlink("/calc_thread");
    if (DEBUG && ret)
        printf("sem_unlink err %d\n", errno);
	ret = sem_unlink("/integration_thread");
    if (DEBUG && ret)
        printf("sem_unlink err %d\n", errno);
	ret = sem_unlink("/exit");
    if (DEBUG && ret)
        printf("sem_unlink err %d\n", errno);
}

static void initialize_semaphores(t_worker *worker)
{
	unlink_semaphores();
	worker->next_tick_sem = sem_open("/next_tick", O_CREAT, 077, 0);
	if (worker->next_tick_sem == SEM_FAILED)
		printf("sem4 open failed with %d\n", errno);
	worker->calc_thread_sem = sem_open("/calc_thread", O_CREAT, 0777, 0);
	if (worker->calc_thread_sem == SEM_FAILED)
		printf("sem1 open failed with %d\n", errno);
	worker->integration_thread_sem = sem_open("/integration_thread", O_CREAT, 0777, 0);
	if (worker->integration_thread_sem == SEM_FAILED)
		printf("sem2 open failed with %d\n", errno);
	worker->exit_sem = sem_open("/exit", O_CREAT, 0777, 0);
	if (worker->exit_sem == SEM_FAILED)
		printf("sem3 open failed with %d\n", errno);
}

int main(int argc, char **argsv)
{
	t_worker	*worker;

	G_softening = SOFTENING;
	G_time_step = TIME_STEP;
	worker = (t_worker *)calloc(1, sizeof(t_worker));
	if (argc == 1)
	{
		printf("Usage ./a.out <inital_dataset.jgrav file>\n");
		exit(1);
	}

	//load inital_dataset
	load_dataset(dispatcher, av[1]);

	initialize_semaphores(worker);
	if (DEBUG)
		printf("semaphores initalized\n");
	launch_simulation_thread(worker);
	launch_calculation_thread(worker);
	launch_integration_thread(worker);
	if (DEBUG)
		printf("threads launched\n");
	int val = sem_wait(worker->exit_sem);
	if (val < 0)
		printf("sem_wait failed with err:%d\n", errno);
	clock_t diff = clock() - G_start_time;
	double sum = G_total_event_time + G_total_calc_time + G_total_send_time;
	if (METRICS)
	{
		printf("Total runtime: %.2lu\n", time(NULL) - G_start_time);
		printf("%-24s %-3ld %-6.2f%%\n", "Time spent eventing:",
				G_total_event_time, (G_total_event_time / sum) * 100);
		printf("%-24s %-3ld %-6.2f%%\n", "Time spent calculating:",
				G_total_calc_time, (G_total_calc_time / sum) * 100);
		printf("%-24s %-3ld %-6.2f%%\n", "Time spent sending:",
				G_total_send_time, (G_total_send_time / sum) * 100);
	}
	printf("worker done, good bye\n");
	//cleanup
	unlink_semaphores();
	return (0);
}
