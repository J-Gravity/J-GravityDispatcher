/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   worker.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 16:08:11 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/14 20:43:43 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
int		btoi(char *str)
{
	int				ret;

	for (unsigned int i = 0; i < sizeof(int); i++)
		((char *)(&ret))[i] = str[i];
	return (ret);
}

float	btof(char *str)
{
	float	ret;

	for (unsigned int i = 0; i < sizeof(float); i++)
		((char *)(&ret))[i] = str[i];
	return (ret);
}

cl_float4	btoclf(char *str)
{
	cl_float4	ret;

	for (unsigned int i = 0; i < sizeof(cl_float4); i++)
		((char *)(&ret))[i] = str[i];
	return (ret);
}

t_workunit	*deserialize_workunit(t_msg msg)
{
	int			i;
	t_workunit	*workunit;

	i = 0;
	workunit = malloc(sizeof(t_workunit));
	workunit->id = btoi(msg.data);
	workunit->localcount = btoi(msg.data += 4);
	workunit->local_bodies = (t_body *)malloc(sizeof(t_body) * workunit->localcount);
	for (int i = 0; i < workunit->localcount; i++)
	{
		workunit->local_bodies[i].position.x = btof(msg.data += 4);
		workunit->local_bodies[i].position.y = btof(msg.data += 4);
		workunit->local_bodies[i].position.z = btof(msg.data += 4);
		workunit->local_bodies[i].velocity.x = btof(msg.data += 4);
		workunit->local_bodies[i].velocity.y = btof(msg.data += 4);
		workunit->local_bodies[i].velocity.z = btof(msg.data += 4);
	}
	workunit->neighborcount = btoi(msg.data += 4);
	workunit->neighborhood = (t_body *)malloc(sizeof(t_body) * workunit->neighborcount);
	for (int i = 0; i < workunit->neighborcount; i++)
	{
		workunit->neighborhood[i].position.x = btof(msg.data += 4);
		workunit->neighborhood[i].position.y = btof(msg.data += 4);
		workunit->neighborhood[i].position.z = btof(msg.data += 4);
		workunit->neighborhood[i].velocity.x = btof(msg.data += 4);
		workunit->neighborhood[i].velocity.y = btof(msg.data += 4);
		workunit->neighborhood[i].velocity.z = btof(msg.data += 4);
	}
	workunit->force_bias = btoclf(msg.data += 4);
	return (workunit);
}