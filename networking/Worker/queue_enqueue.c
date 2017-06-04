/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_enqueue.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:21:21 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/03 17:25:50 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

t_lst	*queue_enqueue(t_queue **queue, t_lst *new)
{
	t_lst	*node;

	if (NULL == (*queue))
		(*queue) = (t_queue *)calloc(1, sizeof(t_queue));
	if (NULL == (*queue)->first)
	{
		(*queue)->first = new;
		(*queue)->first->next = NULL;
		return (new);
	}
	if (NULL == (*queue)->last)
	{
		(*queue)->last = new;
		return (new);
	}
	node = (*queue)->last;
	node->next = new;
	(*queue)->last = node;
	return ((*queue)->last);
}

