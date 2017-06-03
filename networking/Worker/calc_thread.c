/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   calc_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:29:42 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/02 19:12:23 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

static void *calc_thread(void *param)
{
	t_worker	*worker;
    t_workunit  *workunit;

	worker = (t_worker *)param;
    while (1)
    {
        if (worker->todo_work->count > 0)
        {
            workunit = queue_pop(&worker->todo_work);
            workunit = do_workunit(workunit);
            queue_enqueue(&worker->completed_work, queue_create_new(workunit));
        }
    }
    return (0);
}

void launch_calculation_thread(t_worker *worker)
{
    pthread_create(worker->event_thread, NULL, calc_thread, worker);
}
