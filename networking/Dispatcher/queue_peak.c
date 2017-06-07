/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_peak.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iwagner <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/05 20:20:46 by iwagner           #+#    #+#             */
/*   Updated: 2017/06/05 20:20:50 by iwagner          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_workunit	*queue_peak(t_queue **queue)
{
	t_workunit	*workunit;
	t_lst		*node;

	if (*queue)
	{
		pthread_mutex_lock(&(*queue)->mutex);
		node = (*queue)->first;
		if (node)
			workunit = node->data;
		pthread_mutex_unlock(&(*queue)->mutex);
	}
	return (workunit);
}
