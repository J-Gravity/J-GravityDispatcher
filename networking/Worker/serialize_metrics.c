/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serialize_metrics.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/14 15:35:20 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/20 20:23:30 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

t_msg	serialize_metrics(void)
{
	t_msg	msg;
	char	*localblob;
	long	total_time = time(NULL) - G_start_time;
	int		id = 9;
	int		id2;
	
	msg.size = (sizeof(int) + sizeof(long) * 5);
	msg.data = malloc(msg.size);
	int offset = 0;
	memcpy(msg.data, &(id), sizeof(int));
	offset += sizeof(int);
	memcpy(&(id2), msg.data, sizeof(int));
	printf("id2=%d\n", id2);
	memcpy(msg.data, &(total_time), sizeof(long));
	offset += sizeof(long);
	memcpy(msg.data + offset, &(G_time_waiting_for_wu), sizeof(long));
	offset += sizeof(long);
	memcpy(msg.data + offset, &(G_total_event_time), sizeof(long));
	offset += sizeof(long);
	memcpy(msg.data + offset, &(G_total_calc_time), sizeof(long));
	offset += sizeof(long);
	memcpy(msg.data + offset, &(G_total_send_time), sizeof(long));
	offset += sizeof(long);
	msg.id = METRICS_DONE;
	return (msg);
}
