/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deserialize_workunit.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:22:04 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/13 21:09:31 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
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
	WU.neighborcount = 0;
	WU.neighborhood = NULL;
	memcpy(&(WU.id), msg.data, sizeof(int));
	offset += sizeof(int);
	memcpy(&(WU.localcount), msg.data + offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&(local_compressed_size), msg.data + offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&(WU.is_last), msg.data + offset, sizeof(char));
	offset += sizeof(char);
	WU.local_bodies = decompress_locals(msg.data + offset, local_compressed_size, WU.localcount);
	offset += sizeof(t_body) * WU.localcount;
	return (WU);
}
