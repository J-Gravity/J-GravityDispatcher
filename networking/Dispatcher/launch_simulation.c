/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/29 01:30:04 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

static void	setup_semaphores(t_dispatcher *dispatcher)
{
	if (sem_unlink("/exit") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	if (sem_unlink("/sender_thread") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	dispatcher->start_sending = sem_open("/sender_thread", O_CREAT, 0777, 0);
	if (dispatcher->start_sending == SEM_FAILED)
		printf("start_sending sem open failed with %d\n", errno);
	dispatcher->exit_sem = sem_open("/exit", O_CREAT, 0777, 0);
	if (dispatcher->exit_sem == SEM_FAILED)
		printf("sem3 open failed with %d\n", errno);
}

static void	cleanup_semaphores(t_dispatcher *dispatcher)
{
	if (sem_unlink("/sender_thread") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	if (sem_unlink("/exit") && DEBUG)
		printf("sem_unlink err %d\n", errno);
}

static int	sim_ready_prompt()
{
	printf("\rPress \x1b[32m[ENTER] \x1b[0mto start dispatching workunits\n");
	write(1, "[2K", 4);
	write(1, "\rWaiting for workers to connect...\n", 35);
	return (0);
}

void		launch_simulation(t_dispatcher *dispatcher)
{
	setup_semaphores(dispatcher);
	setup_async_file(dispatcher);
	if (DEBUG)
		printf("begin launch_simulation\n");
	if (sim_ready_prompt() == -1)
		return ;
	while (getchar() != 10)
		;
	dispatcher->is_running = 1;
	G_tick_start = time(NULL);
	setup_async_file(dispatcher);
	start_sender_threads(dispatcher, queue_count(dispatcher->workers));
	divide_dataset(dispatcher);
	printf("Simulation Started\n");
	if (sem_wait(dispatcher->exit_sem) < 0)
		printf("sem_wait failed with err:%d\n", errno);
	cleanup_semaphores(dispatcher);
	printf("dispatcher done, good bye\n");
}
