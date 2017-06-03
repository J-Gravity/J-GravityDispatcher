/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sender_thread.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 19:20:34 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/02 20:09:36 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void *sender_thread(void *param)
{
	t_worker	*worker;
    t_workunit  *workunit;
    t_msg       msg;

	worker = (t_worker *)param;
    while (1)
    {
	    pthread_mutex_lock(&worker->sender_thread_mutex);
        while (worker->completed_work->count > 0)// todo: add queue function for getting the counter
        {
            workunit = queue_pop(&worker->completed_work);
            //send complete work unit
	    	msg = serialize_workunit(*workunit);
    		send_msg(worker->socket.fd, msg);
		    if (workunit->local_bodies)
                free(workunit->local_bodies);
            free(workunit);
        }
    }
    return (0);
}

void launch_sender_thread(t_worker *worker)
{
    pthread_create(worker->sender_thread, NULL, sender_thread, worker);
}