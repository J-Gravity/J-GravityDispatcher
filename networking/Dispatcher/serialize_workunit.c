/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serialize_workunit.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:21:10 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/17 22:46:13 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int wu_size(t_workunit w)
{
	int total = 12; //id, localcount, neighborcount
	total += w.localcount * sizeof(t_body);
	total += w.neighborcount * sizeof(t_body);
	total += sizeof(cl_float4);
	return total;
}

t_msg serialize_workunit2(t_workunit w)
{
	t_msg msg;

	msg.data = calloc(1, wu_size(w));
	int offset = 0;
	memcpy(msg.data, &(w.id), sizeof(int));
	offset += sizeof(int);
	memcpy(msg.data + offset, &(w.localcount), sizeof(int));
	offset += sizeof(int);
	memcpy(msg.data + offset, w.local_bodies, sizeof(t_body) * w.localcount);
	offset += sizeof(t_body) * w.localcount;
	memcpy(msg.data + offset, &(w.neighborcount), sizeof(int));
	offset += sizeof(int);
	memcpy(msg.data + offset, w.neighborhood, sizeof(t_body) * w.neighborcount);
	offset += sizeof(t_body) * w.neighborcount;
	memcpy(msg.data + offset, &(w.force_bias), sizeof(cl_float4));
	offset += sizeof(cl_float4);
	//printf("these should be the same: %d, %d\n", wu_size(w), offset);
	msg.size = wu_size(w);

	return (msg);
}