#include "worker.h"
#include "lz4.h"
#include "transpose.h"

t_msg decompress_message(t_msg m)
{
    //something is wrong with either this or the equivalent on dispatch side 
    t_msg dc;
    int decomp_len;
    memcpy(&decomp_len, m.data, sizeof(int));
    char *decompressed = calloc(1, decomp_len);
    LZ4_decompress_safe(m.data + sizeof(int), decompressed, m.size - sizeof(int), decomp_len);
    char *detransposed = calloc(1, decomp_len);
    tpdec((unsigned char *)decompressed, decomp_len, (unsigned char *)detransposed, sizeof(int));
    dc.size = decomp_len;
    dc.data = detransposed;
    free(decompressed);
    return dc;
}

t_bundle *deserialize_bundle(t_msg m)
{
//	m = decompress_message(m);
	// printf("decompressed, m.size is %d\n", m.size);
    t_bundle *b = calloc(1, sizeof(t_bundle));
    size_t offset = 0;

    memcpy(&(b->idcount), m.data + offset, sizeof(int));
	offset += sizeof(int);
    memcpy(&(b->cellcount), m.data + offset, sizeof(int));
    offset += sizeof(int);
    b->ids = calloc(b->idcount, sizeof(int));
    b->local_counts = calloc(b->idcount, sizeof(int));
    b->locals = calloc(b->idcount, sizeof(t_body *));
    for (int i = 0; i < b->idcount; i++)
    {
        memcpy(&(b->ids[i]), m.data + offset, sizeof(int));
        offset += sizeof(int);
        memcpy(&(b->local_counts[i]), m.data + offset, sizeof(int));
        offset += sizeof(int);
	   	b->locals[i] = calloc(b->local_counts[i], sizeof(t_body));
		memcpy(b->locals[i], m.data + offset, b->local_counts[i] * sizeof(t_body));
        offset += b->local_counts[i] * sizeof(t_body);
    }
    b->matches_counts = calloc(b->cellcount, sizeof(int));
    b->matches = calloc(b->cellcount, sizeof(int *));
    b->cell_sizes = calloc(b->cellcount, sizeof (int));
    b->cells = calloc(b->cellcount, sizeof(cl_float4 *));
    for (int i = 0; i < b->cellcount; i++)
    {
        memcpy(&(b->matches_counts[i]), m.data + offset, sizeof(int));
        offset += sizeof(int);
        b->matches[i] = calloc(b->matches_counts[i], sizeof(int));
        memcpy(b->matches[i], m.data + offset, b->matches_counts[i] * sizeof(int));
        offset += b->matches_counts[i] * sizeof(int);
        memcpy(&(b->cell_sizes[i]), m.data + offset, sizeof(int));
        offset += sizeof(int);
        b->cells[i] = calloc(b->cell_sizes[i], sizeof(cl_float4));
        memcpy(b->cells[i], m.data + offset, b->cell_sizes[i] * sizeof(cl_float4));
        offset += b->cell_sizes[i] * sizeof(cl_float4);
    }
    //print some bundle details
    return (b);
}

void transpose_matches(t_bundle *wb)
{
	printf("tm0\n");
    int **manifests = calloc(wb->idcount, sizeof(int *));
	printf("tm1\n");
    int *manifest_lens = calloc(wb->idcount, sizeof(int));
	printf("tm2\n");
	printf("wb->cellcount: %d\n", wb->cellcount);
    for (int i = 0; i < wb->cellcount; i++)
	{
		printf("wb->matches_counts[%d]: %d\n", i, wb->matches_counts[i]);
		for (int j = 0; j < wb->matches_counts[i]; j++)
		{
			manifest_lens[wb->matches[i][j]]++;
		}
	}
	printf("tm3\n");
    for (int i = 0; i < wb->idcount; i++)
    {
		printf("tm4\n");
        manifests[i] = calloc(manifest_lens[i], sizeof(int));
		printf("tm5\n");
        manifest_lens[i] = 0;
    }
    for (int i = 0; i < wb->cellcount; i++)
    {
		printf("tm6\n");
        for (int j = 0; j < wb->matches_counts[i]; j++)
        {
			printf("tm7\n");
            manifests[wb->matches[i][j]][manifest_lens[wb->matches[i][j]]] = i;
			printf("tm8\n");
            manifest_lens[wb->matches[i][j]]++;
        }
    }
//	for (int i = 0; i < wb->cellcount; i++)
//		free(wb->matches[i]);
	printf("tm9\n");
  //  free(wb->matches);
	printf("tm10\n");
  //  free(wb->matches_counts);
	printf("tm11\n");
    wb->matches = manifests;
    wb->matches_counts = manifest_lens;
}

size_t nearest_mult_256(size_t n)
{
    return (((n / 256) + 1) * 256);
}

t_workunit **unbundle_workunits(t_bundle *b, int *count)
{
	printf("uw1\n");
    t_workunit **WUs = calloc(b->idcount, sizeof(t_workunit *));
	printf("uw2\n");
    transpose_matches(b);
	printf("uw3\n");
	unsigned long totalsize = 0;
    for (int i = 0; i < b->idcount; i++)
    {
        WUs[i] = calloc(1, sizeof(t_workunit));
        WUs[i]->id = b->ids[i];
        WUs[i]->localcount = b->local_counts[i];
        WUs[i]->npadding = nearest_mult_256(WUs[i]->localcount) - WUs[i]->localcount;
        WUs[i]->N = calloc(nearest_mult_256(WUs[i]->localcount), sizeof(cl_float4));
        WUs[i]->V = calloc(nearest_mult_256(WUs[i]->localcount), sizeof(cl_float4));
        for(int j = 0; j < WUs[i]->localcount; j++)
        {
            WUs[i]->N[j] = b->locals[i][j].position;
            WUs[i]->V[j] = b->locals[i][j].velocity;
        }
  //      free(b->locals[i]);
        totalsize += WUs[i]->localcount * sizeof(t_body);
        WUs[i]->neighborcount = 0;
        for (int j = 0; j < b->matches_counts[i]; j++)
            WUs[i]->neighborcount += b->cell_sizes[b->matches[i][j]];
        WUs[i]->mpadding = nearest_mult_256(WUs[i]->neighborcount) - WUs[i]->neighborcount;
        WUs[i]->M = (cl_float4 *)calloc(nearest_mult_256(WUs[i]->neighborcount), sizeof(cl_float4));
        totalsize += WUs[i]->neighborcount * sizeof(cl_float4);
        size_t offset = 0;
        for (int j = 0; j < b->matches_counts[i]; j++)
        {
            memcpy(WUs[i]->M + offset, b->cells[b->matches[i][j]], b->cell_sizes[b->matches[i][j]] * sizeof(cl_float4));
            offset += b->cell_sizes[b->matches[i][j]];
        }
    }
    printf("bundle turned into %lu MB\n", totalsize / (1024 * 1024));
    *count = b->idcount;
    return (WUs);
}
