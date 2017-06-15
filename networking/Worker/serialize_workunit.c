 /* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serialize_workunit.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 17:21:10 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/14 23:53:18 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"
#include "lz4.h"
#include "transpose.h"

char *compress_locals(t_workunit w, int *loclen)
{
	//fill a buffer with all the local bodies.
	char *uncompressed = malloc(w.localcount * sizeof(t_body));
	memcpy(uncompressed, w.local_bodies, sizeof(t_body) * w.localcount);

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


t_msg serialize_workunit(t_workunit w)
{
	t_msg msg;
	char *localblob;
	int local_compressed_size;

	localblob = compress_locals(w, &local_compressed_size);
	msg.size = (sizeof(int) * 3 + local_compressed_size + sizeof(char));
	msg.data = calloc(1, msg.size);
	size_t offset = 0;
	memcpy(msg.data, &(w.id), sizeof(int));
	offset += sizeof(int);
	memcpy(msg.data + offset, &(w.localcount), sizeof(int));
	offset += sizeof(int);
	memcpy(msg.data + offset, &(local_compressed_size), sizeof(int));
	offset += sizeof(int);
	memcpy(msg.data + offset, &(w.is_last), sizeof(char));
	offset += sizeof(char);
	memcpy(msg.data + offset, localblob, local_compressed_size);
	offset += local_compressed_size;
	msg.id = WORK_UNIT_DONE;
	free(localblob);
	return (msg);
}
