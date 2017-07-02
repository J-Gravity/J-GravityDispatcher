/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_pop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:20:42 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/05 00:08:22 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

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
