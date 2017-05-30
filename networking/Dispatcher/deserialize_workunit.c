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
#include "lz4.h"
#include "transpose.h"

t_body *decompress_locals(char *localblob, int local_compressed_size, int local_count)
{
	char *decompressed = malloc(local_count * sizeof(t_body));
	LZ4_decompress_safe((char *)localblob, (char *)decompressed, local_compressed_size, local_count * sizeof(t_body));
	t_body *output = malloc(local_count * sizeof(t_body));
	tpdec((unsigned char *)decompressed, local_count * sizeof(t_body), (unsigned char *)output, sizeof(t_body));
	free(decompressed);
	return output;
}

t_WU	deserialize_WU(t_msg msg)
{
	t_WU	WU;
	int local_compressed_size;

	int offset = 0;
	memcpy(&(WU.id), msg.data, sizeof(int));
	struct timeval time;
	gettimeofday(&time, NULL);
	printf("%d wu received: %ld\n", WU.id, time.tv_sec);
	offset += sizeof(int);
	memcpy(&(WU.localcount), msg.data + offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&(local_compressed_size), msg.data + offset, sizeof(int));
	offset += sizeof(int);

	WU.neighborhood = (t_body *)calloc(WU.neighborcount, sizeof(t_body));
	memcpy(WU.neighborhood, msg.data + offset, sizeof(t_body) * WU.neighborcount);
	offset += sizeof(t_body) * WU.neighborcount;
	WU.local_bodies = decompress_locals(msg.data + offset, local_compressed_size, WU.localcount);
	offset += sizeof(t_body) * WU.localcount;

	return (WU);
}
