/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deserialize_workunit.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:22:04 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/28 16:02:03 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <sys/time.h>
#include <time.h>

t_WU	deserialize_WU(t_msg msg)
{
	t_WU	WU;

	int offset = 0;
	memcpy(&(WU.id), msg.data, sizeof(int));
	struct timeval time;
	gettimeofday(&time, NULL);
	printf("%d wu received: %ld\n", WU.id, time.tv_sec);
	offset += sizeof(int);
	memcpy(&(WU.localcount), msg.data + offset, sizeof(int));
	offset += sizeof(int);
	WU.local_bodies = (t_body *)calloc(WU.localcount, sizeof(t_body));
	memcpy(WU.local_bodies, msg.data + offset, sizeof(t_body) * WU.localcount);
	offset += sizeof(t_body) * WU.localcount;
	memcpy(&(WU.neighborcount), msg.data + offset, sizeof(int));
	offset += sizeof(int);
	WU.neighborhood = (t_body *)calloc(WU.neighborcount, sizeof(t_body));
	memcpy(WU.neighborhood, msg.data + offset, sizeof(t_body) * WU.neighborcount);
	offset += sizeof(t_body) * WU.neighborcount;
	memcpy(&(WU.wu_calc_time), msg.data + offset, sizeof(long));
	offset += sizeof(long);
	printf("wu_calc_time: %ld\n", WU.wu_calc_time);
	return (WU);
}
