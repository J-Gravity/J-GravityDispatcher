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

#include "worker.h"

t_msg	serialize_workunit(t_workunit workunit)
{
	t_msg	msg;

	msg.data = calloc(1, sizeof(char));
	msg.data[0] = workunit.id;
	msg.size = 4;
	strbjoin(&msg, itob(workunit.localcount), sizeof(int));
	for (int i = 0; i < workunit.localcount; i++)
	{
		strbjoin(&msg, clftob(workunit.local_bodies[i].position), sizeof(float) * 4);
		strbjoin(&msg, clftob(workunit.local_bodies[i].velocity), sizeof(float) * 4);
	}
	strbjoin(&msg, itob(workunit.neighborcount), sizeof(int));
	for (int i = 0; i < workunit.neighborcount; i++)
	{
		strbjoin(&msg, clftob(workunit.neighborhood[i].position), sizeof(float) * 4);
		strbjoin(&msg, clftob(workunit.neighborhood[i].velocity), sizeof(float) * 4);
	}
	strbjoin(&msg, clftob(workunit.force_bias), sizeof(float) * 4);
	return (msg);
}

// typedef struct			s_workunit
// {
// 	int					id;
// 	int					localcount;
// 	int					neighborcount;
// 	t_body				*local_bodies;
// 	t_body				*neighborhood;
// 	cl_float4			force_bias;
// }						t_workunit;

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
	printf("returning %d bodies (localcount)\n", w.localcount);
	memcpy(msg.data + offset, &(w.neighborcount), sizeof(int));
	offset += sizeof(int);
	memcpy(msg.data + offset, w.neighborhood, sizeof(t_body) * w.neighborcount);
	offset += sizeof(t_body) * w.neighborcount;
	memcpy(msg.data + offset, &(w.force_bias), sizeof(cl_float4));
	offset += sizeof(cl_float4);
	printf("these should be the same: %d, %d\n", wu_size(w), offset);
	msg.size = wu_size(w);
	msg.id = WORK_UNIT_DONE;
	return (msg);
}