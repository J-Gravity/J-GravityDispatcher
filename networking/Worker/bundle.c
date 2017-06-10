#include "worker.h"
#include "lz4.h"
#include "transpose.h"

t_msg decompress_message(t_msg m)
{
    //something is wrong with either this or the equivalent on dispatch side
    t_msg dc;
    int decomp_len;
    memcpy(&decomp_len, m.data, sizeof(int));
    printf("going to decomp to len %d, incoming message is %d\n", decomp_len, m.size);
    char *decompressed = calloc(1, decomp_len);
    LZ4_decompress_safe(m.data + sizeof(int), (char *)decompressed, m.size - sizeof(int), decomp_len);
    char *detransposed = calloc(1, decomp_len);
    tpdec((unsigned char *)decompressed, m.size - sizeof(int), (unsigned char *)detransposed, sizeof(int));
    dc.size = decomp_len;
    dc.data = detransposed;
    free(decompressed);
    free(m.data);
    return dc;
}

t_bundle *deserialize_bundle(t_msg m)
{

    // m = decompress_message(m);
    // printf("decompressed, m.size is %d\n", m.size);
    t_bundle *b = calloc(1, sizeof(t_bundle));
    int offset = 0;

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
    printf("incoming bundle msg was %d MB\n", m.size / (1024 * 1024));
    return (b);
}

void transpose_matches(t_bundle *wb)
{
    int **manifests = calloc(wb->idcount, sizeof(int *));
    int *manifest_lens = calloc(wb->idcount, sizeof(int));
    for (int i = 0; i < wb->cellcount; i++)
        for (int j = 0; j < wb->matches_counts[i]; j++)
            manifest_lens[wb->matches[i][j]]++;
    for (int i = 0; i < wb->idcount; i++)
    {
        manifests[i] = calloc(manifest_lens[i], sizeof(int));
        manifest_lens[i] = 0;
    }
    for (int i = 0; i < wb->cellcount; i++)
    {
        for (int j = 0; j < wb->matches_counts[i]; j++)
        {
            manifests[wb->matches[i][j]][manifest_lens[wb->matches[i][j]]] = i;
            manifest_lens[wb->matches[i][j]]++;
        }
    }
	for (int i = 0; i < wb->cellcount; i++)
		free(wb->matches[i]);
    free(wb->matches);
    free(wb->matches_counts);
    wb->matches = manifests;
    wb->matches_counts = manifest_lens;
}

t_workunit **unbundle_workunits(t_bundle *b, int *count)
{
    t_workunit **WUs = calloc(b->idcount, sizeof(t_workunit *));
    transpose_matches(b);
    unsigned long totalsize = 0;
    for (int i = 0; i < b->idcount; i++)
    {
        WUs[i] = calloc(1, sizeof(t_workunit));
        WUs[i]->id = b->ids[i];
        WUs[i]->localcount = b->local_counts[i];
        WUs[i]->local_bodies = b->locals[i];
        totalsize += WUs[i]->localcount * sizeof(t_body);
        WUs[i]->neighborcount = 0;
        for (int j = 0; j < b->matches_counts[i]; j++)
            WUs[i]->neighborcount += b->cell_sizes[b->matches[i][j]];
        WUs[i]->neighborhood = (cl_float4 *)calloc(WUs[i]->neighborcount, sizeof(cl_float4));
        totalsize += WUs[i]->neighborcount * sizeof(cl_float4);
        int offset = 0;
        for (int j = 0; j < b->matches_counts[i]; j++)
        {
            memcpy(WUs[i]->neighborhood + offset, b->cells[b->matches[i][j]], b->cell_sizes[b->matches[i][j]] * sizeof(cl_float4));
            offset += b->cell_sizes[b->matches[i][j]];
        }
    }
    printf("bundle turned into %lu MB\n", totalsize / (1024 * 1024));
    *count = b->idcount;
    return (WUs);
}

// t_workunit **unbundle_workunits(t_bundle *b, int *count)
// {
//     t_workunit **WUs = calloc(b->idcount, sizeof(t_workunit *));
//     transpose_matches(b);
//     for (int i = 0; i < b->idcount; i++)
//     {
//         WUs[i] = calloc(1, sizeof(t_workunit));
//         WUs[i]->id = b->ids[i];
//         WUs[i]->localcount = b->local_counts[i];
//         WUs[i]->local_bodies = b->locals[i];
//         WUs[i]->neighborcount = 0;
//         for (int j = 0; j < b->matches_counts[i]; j++)
//             WUs[i]->neighborcount += b->cell_sizes[b->matches[i][j]];
//         WUs[i]->neighborhood = (cl_float4 **)calloc(b->matches_counts[i] + 1, sizeof(cl_float4 *));
//         WUs[i]->cell_lens = (int *)calloc(b->matches_counts[i], sizeof(int));
//         for (int j = 0; j < b->matches_counts[i]; j++)
//         {
//             WUs[i]->neighborhood[j] = b->cells[b->matches[i][j]];
//             WUs[i]->cell_lens[j] = b->cell_sizes[b->matches[i][j]];
//         }
//     }
//     *count = b->idcount;
//     return (WUs);
// }