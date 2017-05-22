/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   make_new_event_thread.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/22 13:37:17 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/22 14:09:11 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <pthread.h>

void	make_new_event_thread(t_dispatcher *disp, t_lst *worker_link)
{ 
	t_thread_handler	*params;
	t_worker			*worker;

	params = new_thread_handler(disp, worker_link);
	worker = (t_worker *)worker_link->data;
	pthread_create(worker->tid, NULL,
					handle_worker_connection, params);
}