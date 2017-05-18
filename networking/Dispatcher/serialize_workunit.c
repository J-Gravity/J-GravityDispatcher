/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serialize_workunit.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:21:10 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/17 21:43:26 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

char	*itob(int value)
{
	char	*string;

	string = calloc(1, sizeof(int));
	for (unsigned int i = 0; i < sizeof(int); i++)
	{
		string[i] = ((char *)(&value))[i];
		i++;
	}
	return string;	
}

char	*clftob(cl_float4 star)
{
	char	*string;

	string = calloc(1, sizeof(cl_float4));
	for (unsigned int i = 0; i < sizeof(cl_float4); i++)
		string[i] = ((char *)(&star))[i];
	return string;
}

void	strbjoin(t_msg *msg, char const *s2, size_t size)
{
	char	*copy;
	int		i;

	i = 0;
	copy = (char *)malloc(msg->size);
	for (int x = 0; x < msg->size; x++)
		copy[x] = msg->data[x];
	msg->data = (char *)calloc(1, msg->size + size);
	while (i < msg->size)
	{
		msg->data[i] = copy[i];
		i++;
	}
	for (unsigned int x = 0; x < size; x++)
		msg->data[i + x] = s2[x];
	msg->size += size;
}

t_msg	serialize_workunit(t_workunit *workunit)
{
	t_msg	msg;

	msg.data = calloc(1, sizeof(char));
	msg.data[0] = workunit->id;
	msg.size = 4;
	strbjoin(&msg, itob(workunit->localcount), sizeof(int));
	for (int i = 0; i < workunit->localcount; i++)
	{
		strbjoin(&msg, clftob(workunit->local_bodies[i].position), sizeof(float) * 4);
		strbjoin(&msg, clftob(workunit->local_bodies[i].velocity), sizeof(float) * 4);
	}
	strbjoin(&msg, itob(workunit->neighborcount), sizeof(int));
	for (int i = 0; i < workunit->neighborcount; i++)
	{
		strbjoin(&msg, clftob(workunit->neighborhood[i].position), sizeof(float) * 4);
		strbjoin(&msg, clftob(workunit->neighborhood[i].velocity), sizeof(float) * 4);
	}
	strbjoin(&msg, clftob(workunit->force_bias), sizeof(float) * 4);
	return (msg);
}
