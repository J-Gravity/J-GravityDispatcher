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
#include "lz4.h"
#include "transpose.h"

t_body *decompress_locals(char *localblob, int local_compressed_size, int local_count)
{
	char *decompressed = malloc(local_count * sizeof(t_body));
	LZ4_decompress_safe((char *)localblob, (char *)decompressed, local_compressed_size, local_count * sizeof(t_body));
	t_body *output = malloc(local_count * sizeof(t_body));
	tpdec((unsigned char *)decompressed, local_count * sizeof(t_body), (unsigned char *)output, sizeof(t_body));
	free(decompressed);
	free(localblob);
	return output;
}

cl_float4 *decompress_neighbors(char *neighborblob, int neighbor_compressed_size, int neighbor_count)
{
	char *decompressed = malloc(neighbor_count * sizeof(cl_float4));
	LZ4_decompress_safe((char *)neighborblob, (char *)decompressed, neighbor_compressed_size, neighbor_count * sizeof(cl_float4));
	cl_float4 *output = malloc(neighbor_count * sizeof(cl_float4));
	tpdec((unsigned char *)decompressed, neighbor_count * sizeof(cl_float4), (unsigned char *)output, sizeof(cl_float4));
	free(decompressed);
	free(neighborblob);
	return output;
}

t_workunit deserialize_workunit(t_msg msg)
{
	t_workunit w;

	int offset = 0;
	int neighbor_compressed_size;
	int local_compressed_size;

	memcpy(&(w.id), msg.data, sizeof(int));
	offset += sizeof(int);
	
	memcpy(&(w.localcount), msg.data + offset, sizeof(int));
	offset += sizeof(int);
	
	memcpy(&local_compressed_size, msg.data + offset, sizeof(int));
	offset += sizeof(int);
	
	memcpy(&(w.neighborcount), msg.data + offset, sizeof(int));
	offset += sizeof(int);

	memcpy(&neighbor_compressed_size, msg.data + offset, sizeof(int));
	offset += sizeof(int);
	
	char *localblob = malloc(local_compressed_size);
	memcpy(localblob, msg.data + offset, local_compressed_size);
	offset += local_compressed_size;
	w.local_bodies = decompress_locals(localblob, local_compressed_size, w.localcount);

	char *neighborblob = malloc(neighbor_compressed_size);
	memcpy(neighborblob, msg.data + offset, neighbor_compressed_size);
	offset += neighbor_compressed_size;
	w.neighborhood = decompress_neighbors(neighborblob, neighbor_compressed_size, w.localcount);

	// if (offset == msg.size)
	// 	printf("size matched as expected\n");
	// else
	// 	printf("size mismatch, wtf\n");

	printf("the first neighbor body of unit %d has position %f %f %f %f\n", w.id, w.neighborhood[0].x, w.neighborhood[0].y, w.neighborhood[0].z, w.neighborhood[0].w);


	return (w);
}