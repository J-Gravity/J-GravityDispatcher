/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_count.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iwagner <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/05 20:21:03 by iwagner           #+#    #+#             */
/*   Updated: 2017/06/05 20:21:07 by iwagner          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int	*queue_count(t_queue **queue)
{
	int x;

	if (*queue)
	{
		pthread_mutex_lock(&(*queue)->mutex);
		if((x = ((*queue)->count)))
		{
			pthread_mutex_unlock(&(*queue)->mutex);
			return(x);
		}
		else
		{
			pthread_mutex_unlock(&(*queue)->mutex);
			return(0);
		}
}
