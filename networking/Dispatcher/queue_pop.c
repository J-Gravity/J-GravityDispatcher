/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_pop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:20:42 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/03 21:47:00 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_workunit	*queue_pop(t_queue **queue)
{
	t_workunit	*workunit;
	t_lst		*node;

	if (*queue)
	{
		pthread_mutex_lock(&(*queue)->mutex);
		node = (*queue)->first;
		if (node)
		{
			workunit = node->data;
			(*queue)->first = (*queue)->first->next;
			free(node);
		}
		pthread_mutex_unlock(&(*queue)->mutex);
	}
	return (workunit);
}
