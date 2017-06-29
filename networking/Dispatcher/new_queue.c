/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_queue.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/28 22:46:51 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/28 22:48:45 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_queue	*new_queue()
{
	t_queue *queue;

	queue = (t_queue *)calloc(1, sizeof(t_queue));
	pthread_mutex_init(&queue->mutex, NULL);
	return (queue);
}
