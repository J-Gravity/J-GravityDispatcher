/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_enqueue.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:21:21 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/04 17:31:07 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_lst	*queue_enqueue(t_queue **queue, t_lst *new)
{
	t_lst	*node;

	if (NULL == (*queue))
	{
		(*queue) = (t_queue *)calloc(1, sizeof(t_queue));
		pthread_mutex_init(&(*queue)->mutex, NULL);
	}
	pthread_mutex_lock(&((*queue)->mutex));
	(*queue)->count++;
	if (NULL == (*queue)->first)
	{
		(*queue)->first = new;
		(*queue)->first->next = NULL;
		pthread_mutex_unlock(&(*queue)->mutex);
		return (new);
	}
	if (NULL == (*queue)->last)
	{
		(*queue)->last = new;
		(*queue)->last->next = NULL;
		(*queue)->first->next = (*queue)->last;
		pthread_mutex_unlock(&(*queue)->mutex);
		return (new);
	}
	(*queue)->last->next = new;
    (*queue)->last = (*queue)->last->next;
	pthread_mutex_unlock(&((*queue)->mutex));
	return ((*queue)->last);
}

