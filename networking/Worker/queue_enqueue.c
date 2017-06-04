/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_enqueue.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:21:21 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/03 20:44:21 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

t_lst	*queue_enqueue(t_queue **queue, t_lst *new)
{
	t_lst	*node;

	if (NULL == (*queue))
		(*queue) = (t_queue *)calloc(1, sizeof(t_queue));
	(*queue)->count++;
	if (NULL == (*queue)->first)
	{
		(*queue)->first = new;
		(*queue)->first->next = NULL;
		return (new);
	}
	if (NULL == (*queue)->last)
	{
		(*queue)->last = new;
		(*queue)->last->next = NULL;
		(*queue)->first->next = (*queue)->last;
		return (new);
	}
	node = (*queue)->last;
	node->next = new;
	(*queue)->last = node;
	return ((*queue)->last);
}

