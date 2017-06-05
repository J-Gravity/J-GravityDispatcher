/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_pop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:20:42 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/04 16:34:02 by cyildiri         ###   ########.fr       */
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
		(*queue)->count--;
		node = (*queue)->first;
		if (node)
		{
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
