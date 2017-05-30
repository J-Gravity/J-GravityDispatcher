/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serialize_workunit.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:21:10 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/28 15:40:34 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int wu_size(t_workunit w)
{
	int total = 12; //id, localcount, neighborcount
	total += w.localcount * sizeof(t_body);
	total += w.neighborcount * sizeof(cl_float4);
	return total;
}

t_msg serialize_workunit(t_workunit w)
{
	t_msg msg;

	msg.data = calloc(1, wu_size(w));
	int offset = 0;
	memcpy(msg.data, &(w.id), sizeof(int));
	offset += sizeof(int);
	memcpy(msg.data + offset, &(w.localcount), sizeof(int));
	offset += sizeof(int);
	for (int i = 0; i < w.localcount; i++, offset += sizeof(t_body))
		memcpy(msg.data + offset, w.local_bodies[i], sizeof(t_body));
	memcpy(msg.data + offset, &(w.neighborcount), sizeof(int));
	offset += sizeof(int);
	for (int i = 0; i < w.neighborcount; i++, offset += sizeof(cl_float4))
	{
		if (w.neighborhood[i] != NULL)
			memcpy(msg.data + offset, &(w.neighborhood[i]->position), sizeof(cl_float4));
		if (w.neighborhood[i] && w.neighborhood[i]->velocity.w == -1)
		{
			free(w.neighborhood[i]);
			w.neighborhood[i] = NULL;
		}
	}
//	w.wu_calc_time = 0;
//	memcpy(msg.data, &(w.wu_calc_time), sizeof(long));
	msg.size = wu_size(w);
	return (msg);
}
