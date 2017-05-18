/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   launch_simulation.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/11 20:53:00 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/17 21:38:31 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include "pthread.h"

static void	*handle_worker_connection(void *input);

static void	handle_worker_msg(t_dispatcher *dispatcher, t_worker *worker,
			t_msg msg)
{
	if (msg.id == WORK_UNIT_REQUEST)
		handle_workunit_req(dispatcher, worker, msg);
	else if (msg.id == WORK_UNIT_DONE)
		handle_worker_done_msg(dispatcher, worker, msg);
	else
		printf("invalid msg id: %d", msg.id);
}

static void	make_new_event_thread(t_dispatcher *disp, t_lst *worker_link)
{
	t_thread_handler	*params;
	t_worker			*worker;

	params = new_thread_handler(disp, worker_link);
	worker = (t_worker *)worker_link->data;
	pthread_create(worker->tid, NULL,
					handle_worker_connection, params);
}

static void	*handle_worker_connection(void *input)
{
	t_thread_handler	*params;
	t_lst				*worker;
	t_worker			*cur_worker;
	t_msg				msg;

	params = (t_thread_handler *)input;
	worker = params->worker;
	cur_worker = (t_worker *)worker->data;
	send_worker_msg(cur_worker, new_message(WORK_UNITS_READY, 0, ""));
	while (1)
	{
		if (worker->next && ((t_worker *)worker->next)->tid == 0)
		{
			make_new_event_thread(params->dispatcher, worker->next);
		}
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
	pthread_create(cur_worker->tid, NULL,
					handle_worker_connection, param);
}
