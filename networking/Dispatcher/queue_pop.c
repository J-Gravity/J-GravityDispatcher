/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_pop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:20:42 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/13 16:54:17 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_lst	*queue_pop_link(t_queue **queue)
{
	t_lst		*node;

	node = NULL;
	if (*queue)
	{
		pthread_mutex_lock(&(*queue)->mutex);
		if ((*queue)->first)
		{
			node = (*queue)->first;
			(*queue)->count--;
			(*queue)->first = (*queue)->first->next;
		}
		if ((*queue)->count == 0)
			(*queue)->last = NULL;
		pthread_mutex_unlock(&(*queue)->mutex);
	}
	return (node);
}

void	*queue_pop(t_queue **queue)
{
	t_bundle	*bundle;
	t_lst		*node;

	bundle = NULL;
	if (*queue)
	{
		pthread_mutex_lock(&(*queue)->mutex);
		node = (*queue)->first;
		if (node)
		{
			(*queue)->count--;
			bundle = node->data;
			(*queue)->first = (*queue)->first->next;
			free(node);
		}
		if ((*queue)->count == 0)
			(*queue)->last = NULL;
		pthread_mutex_unlock(&(*queue)->mutex);
	}
	return (bundle);
}
