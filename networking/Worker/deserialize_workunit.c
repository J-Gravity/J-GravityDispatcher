/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deserialize_workunit.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:22:04 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/18 15:46:48 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

t_workunit deserialize_workunit(t_msg msg)
{
	t_workunit w;

	clock_t start, end;
	start = clock();

	int offset = 0;
	memcpy(&(w.id), msg.data, sizeof(int));
	offset += sizeof(int);
	memcpy(&(w.localcount), msg.data + offset, sizeof(int));
	offset += sizeof(int);
	w.local_bodies = (t_body *)calloc(w.localcount, sizeof(t_body));
	memcpy(w.local_bodies, msg.data + offset, sizeof(t_body) * w.localcount);
	offset += sizeof(t_body) * w.localcount;
	memcpy(&(w.neighborcount), msg.data + offset, sizeof(int));
	offset += sizeof(int);
	w.neighborhood = (cl_float4 *)calloc(w.neighborcount, sizeof(cl_float4));
	memcpy(w.neighborhood, msg.data + offset, sizeof(cl_float4) * w.neighborcount);
	offset += sizeof(cl_float4) * w.neighborcount;
	end = clock() - start;
	printf("DS took %lu for %d\n", end, msg.size);
	if (msg.size != offset)
		printf("SIZE MISMATCH! %d, %d\n", msg.size, offset);
	return (w);
}