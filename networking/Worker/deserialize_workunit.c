/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deserialize_workunit.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:22:04 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/17 22:34:52 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Dispatcher/dispatcher.h"

int			btoi(char *str)
{
	int		ret;

	for (unsigned int i = 0; i < sizeof(int); i++)
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

t_workunit	deserialize_workunit(t_msg msg)
{
	int			i;
	t_workunit	workunit;

	i = 0;
	workunit.id = btoi(msg.data);
	workunit.localcount = btoi(msg.data += 4);
	workunit.local_bodies = (t_body *)malloc(sizeof(t_body) * workunit.localcount);
	for (int i = 0; i < workunit.localcount; i++)
	{
		workunit.local_bodies[i].position = btoclf(msg.data += 16);
		workunit.local_bodies[i].velocity = btoclf(msg.data += 16);
	}
	workunit.neighborcount = btoi(msg.data += 4);
	workunit.neighborhood = (t_body *)malloc(sizeof(t_body) * workunit.neighborcount);
	for (int i = 0; i < workunit.neighborcount; i++)
	{
		workunit.neighborhood[i].position = btoclf(msg.data += 16);
		workunit.neighborhood[i].velocity = btoclf(msg.data += 16);
	}
	workunit.force_bias = btoclf(msg.data += 4);
	return (workunit);
}
