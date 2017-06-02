/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serialize_workunit.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:21:10 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/31 14:21:28 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include "lz4.h"
#include "transpose.h"

char *compress_locals(t_workunit w, int *loclen)
{
	//fill a buffer with all the local bodies.
	char *uncompressed = malloc(w.localcount * sizeof(t_body));
	int offset = 0;
	for (int i = 0; i < w.localcount; i++, offset += sizeof(t_body))
		memcpy(uncompressed + offset, w.local_bodies[i], sizeof(t_body));

	//byte transpose using TurboTranspose
	char *transposed = malloc(w.localcount * sizeof(t_body));
	tpenc((unsigned char *)uncompressed, w.localcount * sizeof(t_body), (unsigned char *)transposed, sizeof(t_body));

	//we cannot predict exactly how big the compressed result will be til we do it. LZ4_compressBound gives an upper bound.
	//allocate that much, compress into it. It returns an int representing how big the result actually was.
	int max_compressed_size = LZ4_compressBound(w.localcount * sizeof(t_body));
	char *compressed = malloc(max_compressed_size);
	//compress with LZ4
	int result_compressed_size = LZ4_compress_default(transposed, compressed, w.localcount * sizeof(t_body), max_compressed_size);
	*loclen = result_compressed_size;
	//printf("locals compressed to %d from %lu, %.f%% of original size\n", result_compressed_size, w.localcount * sizeof(t_body), (float)result_compressed_size * 100.0  / ((float)w.localcount * sizeof(t_body)));
	free(transposed);
	free(uncompressed);
	return(compressed);
}

char *compress_neighbors(t_workunit w, int *neighblen)
{
	//fill a buffer with all the neighbor positions.
	char *uncompressed = malloc(w.neighborcount * sizeof(cl_float4));
	int offset = 0;
	for (int i = 0; i < w.neighborcount; i++, offset += sizeof(cl_float4))
		memcpy(uncompressed + offset, &(w.neighborhood[i]->position), sizeof(cl_float4));

	//byte transpose using TurboTranspose
	char *transposed = malloc(w.neighborcount * sizeof(cl_float4));
	tpenc((unsigned char *)uncompressed, w.neighborcount * sizeof(cl_float4), (unsigned char *)transposed, sizeof(cl_float4));

	//we cannot predict exactly how big the compressed result will be til we do it. LZ4_compressBound gives an upper bound.
	//allocate that much, compress into it. It returns an int representing how big the result actually was.
	int max_compressed_size = LZ4_compressBound(w.neighborcount * sizeof(cl_float4));
	char *compressed = malloc(max_compressed_size);
	//compress with LZ4
	int result_compressed_size = LZ4_compress_default(transposed, compressed, w.neighborcount * sizeof(cl_float4), max_compressed_size);
	//printf("max_c_s %d vs result_c_s %d\n", max_compressed_size, result_compressed_size);
	*neighblen = result_compressed_size;
	//printf("neighbors compressed to %d bytes, originally %lu bytes, %.f%% of original size\n", result_compressed_size, w.neighborcount * sizeof(cl_float4), (float)result_compressed_size * 100.0  / ((float)w.neighborcount * sizeof(cl_float4)));
	free(transposed);
	free(uncompressed);
	return(compressed);
}

t_msg serialize_workunit(t_workunit w)
{
	t_msg msg;

	int local_compressed_size;
	char *localblob = compress_locals(w, &local_compressed_size);

	int neighborhood_compressed_size;
	char *neighborblob = compress_neighbors(w, &neighborhood_compressed_size);

	msg.size = sizeof(int) * 5 + local_compressed_size + neighborhood_compressed_size;
	msg.data = malloc(msg.size);
	int offset = 0;
	
	memcpy(msg.data, &(w.id), sizeof(int));
	offset += sizeof(int);

	memcpy(msg.data + offset, &(w.localcount), sizeof(int));
	offset += sizeof(int);

	memcpy(msg.data + offset, &(local_compressed_size), sizeof(int));
	offset += sizeof(int);

	memcpy(msg.data + offset, &(w.neighborcount), sizeof(int));
	offset += sizeof(int);

	memcpy(msg.data + offset, &(neighborhood_compressed_size), sizeof(int));
	offset += sizeof(int);

	memcpy(msg.data + offset, localblob, local_compressed_size);
	offset += local_compressed_size;

	memcpy(msg.data + offset, neighborblob, neighborhood_compressed_size);
	offset += neighborhood_compressed_size;
	G_workunit_size += offset;

	// if (offset == msg.size)
	// 	printf("sizes matched as expected\n");
	// else
	// 	printf("size mismatch, wtf\n");
	int origsize = w.localcount * sizeof(t_body) + w.neighborcount *sizeof(cl_float4) + 5 * sizeof(int);
	free(localblob);
	free(neighborblob);
	return (msg);
}
