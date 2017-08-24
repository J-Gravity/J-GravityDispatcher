/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_create_new.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:22:08 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/09 03:12:27 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "standalone.h"

t_lst	*queue_create_new(void *workunit)
{
	t_lst	*node;

	node = (t_lst *)calloc(1, sizeof(t_lst));
	node->data = workunit;
	node->next = NULL;
	return (node);
}

void	*queue_pop(t_queue **queue)
{
	void	*workunit;
	t_lst		*node;

	workunit = NULL;
	if (*queue)
	{
		pthread_mutex_lock(&(*queue)->mutex);
		node = (*queue)->first;
		if (node)
		{
			(*queue)->count--;
			workunit = node->data;
			(*queue)->first = (*queue)->first->next;
			free(node);
		}
		if ((*queue)->count == 0)
			(*queue)->last = NULL;
		pthread_mutex_unlock(&(*queue)->mutex);
	}
	return (workunit);
}

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

