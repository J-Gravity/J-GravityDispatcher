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
