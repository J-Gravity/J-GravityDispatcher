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

#include "dispatcher.h"
#include "pthread.h"

static void	handle_worker_msg(t_dispatcher *dispatcher, t_worker *worker,
			t_msg msg)
{
	if (msg.id == WORK_UNIT_REQUEST)
		handle_work_unit_req(dispatcher, worker, msg);
	else if (msg.id == WORKER_DONE)
		handle_worker_done_msg(dispatcher, worker, msg);
	else
		printf("invalid msg id: %d", msg.id);
}

static void	handle_worker_connection(t_thread_handler *params)
{
	t_lst				*worker;
	t_worker			*cur_worker;
	t_worker			*next_worker;
	t_thread_handler	*next_params;
	t_msg				msg;

	worker = params->worker;
	cur_worker = (t_worker *)worker->data;
	send_worker_msg(cur_worker, new_message(WORK_UNITS_READY, 0, ""));
	while (1)
	{
		if (worker->next && ((t_worker *)worker->next)->tid == 0)
		{
			next_params = new_thread_handler(params->dispatcher, worker->next);
			next_worker = (t_worker *)worker->next->data;
			pthread_create(next_worker->tid, NULL,
							handle_worker_connection, next_params);
		}
		//wait for network msg on this worker's conneciton
		msg = get_worker_msg(cur_worker);
		if (msg.error == -1)
			printf("get worker message failed with err %d\n", errno);
		else
			handle_worker_msg(params->dispatcher, cur_worker, msg);
	}
}

void		launch_simulation(t_dispatcher *dispatcher)
{
	t_worker			*cur_worker;
	t_thread_handler	*param;

	param = new_thread_handler(dispatcher, dispatcher->workers);
	cur_worker = (t_worker *)dispatcher->workers->data;
	//launch worker network event threads
	pthread_create(cur_worker->tid, NULL,
					handle_worker_connection, param);
}
