/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sender_thread.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 19:20:34 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/09 03:18:08 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void *sender_thread(void *param)
{
	t_worker	*worker;
	t_workunit	*workunit;
	t_msg		msg;

	worker = (t_worker *)param;
    while (1)
    {
	    if (sem_wait(worker->sender_thread_sem) < 0)
		    printf("SEND- sem_wait failed with err:%d\n", errno);
	   	if (DEBUG)
            printf("SEND- sending work unit\n");
        workunit = queue_pop(&worker->completed_work);
        //send complete work unit
        msg = serialize_workunit(*workunit);
		if (DEBUG)
        	printf("sending: %d\n", workunit->id);
		send_msg(worker->socket.fd, msg);
		free(workunit->local_bodies);
		free(workunit->neighborhood);
        free(workunit);
	   	if (DEBUG)
            printf("SEND- finished sending work unit\n");
    }
    return (0);
}

void launch_sender_thread(t_worker *worker)
{
	worker->sender_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->sender_thread, NULL, sender_thread, worker);
}
