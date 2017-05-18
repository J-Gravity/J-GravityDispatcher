/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deserialize_returnunit.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:22:04 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/17 21:57:42 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

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

t_workunit	*deserialize_returnunit(t_msg msg)
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
		workunit->local_bodies[i].position = btoclf(msg.data += 16);
		workunit->local_bodies[i].velocity = btoclf(msg.data += 16);
	}
	return (workunit);
}
