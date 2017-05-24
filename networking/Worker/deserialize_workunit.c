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
	w.neighborhood = (t_body *)calloc(w.neighborcount, sizeof(t_body));
	memcpy(w.neighborhood, msg.data + offset, sizeof(t_body) * w.neighborcount);
	offset += sizeof(t_body) * w.neighborcount;
	return (w);
}