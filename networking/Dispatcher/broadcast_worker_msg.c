/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   broadcast_worker_msg.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/16 22:38:04 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/16 22:39:04 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	broadcast_worker_msg(t_lst *workers, t_msg msg)
{
	t_lst	*head;

	head = workers;
	while (head)
	{
        send_worker_msg((t_worker *)head->data, msg);
		head = head->next;
	}
	free(msg.data);
}