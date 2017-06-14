/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_peak.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/05 20:20:46 by iwagner           #+#    #+#             */
/*   Updated: 2017/06/12 18:37:22 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	*queue_peak(t_queue **queue)
{
	void	*bundle;
	t_lst		*node;

	if (*queue)
	{
		pthread_mutex_lock(&(*queue)->mutex);
		node = (*queue)->first;
		if (node)
			bundle = node->data;
		pthread_mutex_unlock(&(*queue)->mutex);
	}
	return (bundle);
}
