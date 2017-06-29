/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/29 00:48:29 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include "pthread.h"

static void	handle_worker_msg(t_dispatcher *dispatcher, t_worker *worker,
			t_msg msg)
{
	if (DEBUG && MSG_DEBUG && MSG_DETAILS_DEBUG)
		printf("handling request!\n");
	if (msg.id == WORK_UNIT_REQUEST)	
		handle_workunit_req(dispatcher, worker, msg);
	else if (msg.id == WORK_UNIT_DONE)
	{
		if (METRICS && WORKER_TIME_METRIC)
			G_worker_calcs += time(NULL) - worker->w_calc_time;
		handle_worker_done_msg(dispatcher, worker, msg);
		worker->w_calc_time = 0;
	}
	else
		printf("invalid msg id: %d\n", msg.id);
}

void		*worker_event_thread(void *input)
{
	t_thread_handler	*params;
	t_lst				*worker_link;
	t_worker			*worker;
	t_msg				msg;

	signal(SIGPIPE, SIG_IGN);
	if (DEBUG && WORKER_DEBUG)
		printf("Launched worker network handler thread!\n");
	params = (t_thread_handler *)input;
	worker_link = params->worker;
	worker = (t_worker *)worker_link->data;
	worker->active = 1;
	while (worker->active)
	{
		msg = get_msg(worker->socket.fd);
		if (DEBUG && MSG_DEBUG && MSG_DETAILS_DEBUG)
		{
			printf("done receiving message\n");
			printf("msg status: %d\n", msg.error);
			printf("MSG RECIEVED: [id]=%d", msg.id);
			printf(" size '%zu'\n", msg.size);
			printf(" body '%s'\n", msg.data);
   		}
		if (msg.error == -1)
		{
			printf("get worker message failed with err %d\n", errno);
		}
		if (msg.error == 0 || msg.error == -1)
		{
			if (DEBUG && WORKER_DEBUG)
				printf("worker connection terminated! %d\n", worker->socket.fd);
			worker->active = 0;
		}
		else
			handle_worker_msg(params->dispatcher, worker, msg);
	}
	cleanup_worker(params->dispatcher, worker_link);
	free(params);
	if (DEBUG && WORKER_DEBUG)
		printf("killing this thread...\n");
	return (0);
}

int		sim_ready_prompt(t_dispatcher *dispatcher)
{
	printf("\rPress \x1b[32m[ENTER] \x1b[0mto start dispatching workunits\n");
	write(1, "[2K", 4);
	write(1, "\rWaiting for workers to connect...\n", 35);
	return (0);
}

void		launch_simulation(t_dispatcher *dispatcher)
{
	t_worker			*cur_worker;
	t_thread_handler	*param;
	t_lst				*head;	

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

	setup_async_file(dispatcher);
	if (DEBUG)
		printf("begin launch_simulation\n");
	if (sim_ready_prompt(dispatcher) == -1)
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
	if (sem_unlink("/sender_thread") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	if (sem_unlink("/exit") && DEBUG)
		printf("sem_unlink err %d\n", errno);
	printf("dispatcher done, good bye\n");
}
