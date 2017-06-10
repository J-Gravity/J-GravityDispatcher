/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   event_thread.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 17:06:01 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/09 03:13:26 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

// typedef struct s_bundle
// {
//     int idcount;
//     int *ids;
//     int *matches_counts;
//     int **matches;
//     int *local_counts;
//     t_body **locals;
//     int *cell_sizes;
//     cl_float4 **cells;
//     int cellcount;
// }               t_bundle;

void	delete_bundle(t_bundle *bundle)
{
	if (!bundle)
		return ;
	for (int i = 0; i < bundle->idcount; i++)
	{
		free(bundle->matches[i]);
	}
	free(bundle->matches);
	free(bundle->matches_counts);
	for (int i = 0; i < bundle->cellcount; i++)
		free(bundle->cells[i]);
	free(bundle->cells);
	free(bundle->cell_sizes);
	free(bundle->ids);
	free(bundle->local_counts);
	free(bundle->locals);
	free(bundle);
}

static void	handle_event(t_worker *worker, t_msg msg)
{
	static int total_wu_rec = 0;
	if (msg.id == WORK_UNITS_READY)
	{
		//may need to mutex the socket to avoid conflict with sender_thread
		send_msg(worker->socket.fd, (t_msg){WORK_UNIT_REQUEST, 1, strdup(" ")});
		free(msg.data);
	}
	else if (msg.id == WORK_UNIT)
	{
		int count;
		t_bundle *bundle;
		bundle = deserialize_bundle(msg);
		t_workunit **WUs = unbundle_workunits(bundle, &count);
		delete_bundle(bundle);
		for (int i = 0; i < count; i++)
		{
			queue_enqueue(&worker->todo_work, queue_create_new(WUs[i]));
			total_wu_rec++;
			sem_post(worker->calc_thread_sem);
		}
		free(WUs);
		if (DEBUG)
        	printf("EVENT- work units from bundle added to local queue\n");
		if (DEBUG)
        	printf("%d workunits recieved\n", total_wu_rec);
		free(msg.data);
	}
}

static void	*event_thread(void *param)
{
	t_msg		msg;
	t_worker	*worker;

	worker = (t_worker *)param;
	while (worker->active)
	{
		msg = receive_msg(worker->socket.fd);
		if (DEBUG && MSG_DEBUG && MSG_DETAILS_DEBUG)
		{
			printf("EVENT- done receiving message\n");
			printf("EVENT- msg status: %d\n", msg.error);
			printf("EVENT- MSG RECIEVED: [id]=%d", msg.id);
			printf("EVENT- size '%d'\n", msg.size);
			printf("EVENT- body '%s'\n", msg.data);
		}
		if (msg.error == -1)
			printf("EVENT- get worker message failed with err %d\n", errno);
		if (msg.error == 0 || msg.error == -1)
		{
			if (DEBUG && NETWORK_DEBUG)
				printf("EVENT- dispatcher connection terminated! %d\n", worker->socket.fd);
			worker->active = 0;
		}
		else
			handle_event(worker, msg);
		if (DEBUG)
			printf("EVENT- finished event_thread\n");
	}
	printf("EVENT- exiting event thread\n");
	sem_post(worker->exit_sem);
	return (0);
}

void		launch_event_thread(t_worker *worker)
{
	worker->active = 1;
	worker->event_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->event_thread, NULL, event_thread, worker);
}
