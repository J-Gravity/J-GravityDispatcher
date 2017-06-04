/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_pop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:20:42 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/03 16:57:37 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

t_workunit	*queue_pop(t_queue **queue)
{
	t_workunit	*workunit;
	t_lst		*node;

	node = (*queue)->first;
	if(node)
	{
		workunit = node->data;
		(*queue)->first = (*queue)->first->next;
		free(node);
	}
	return (workunit);
}

