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

size_t nearest_mult_256(size_t n)
{
    return (((n / 256) + 1) * 256);
}

t_bundle *deserialize_bundle(t_msg m)
{
    //m = decompress_message(m);
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
    //free(m.data);
    b->index = 0;
    transpose_matches(b);
    return (b);
}

t_workunit *kick_bundle(t_bundle *b)
{
    static int maxM;
    t_workunit *w = calloc(1, sizeof(t_workunit));
    w = calloc(1, sizeof(t_workunit));
    int i = b->index;
    if (i == 0)
        maxM = 0;
    w->id = b->ids[i];
    w->localcount = b->local_counts[i];
    w->npadding = nearest_mult_256(w->localcount) - w->localcount;
    w->N = calloc(nearest_mult_256(w->localcount), sizeof(cl_float4));
    w->V = calloc(nearest_mult_256(w->localcount), sizeof(cl_float4));
    for(int j = 0; j < w->localcount; j++)
    {
        w->N[j] = b->locals[i][j].position;
        w->V[j] = b->locals[i][j].velocity;
    }
    free(b->locals[i]);
    w->neighborcount = 0;
    for (int j = 0; j < b->matches_counts[i]; j++)
        w->neighborcount += b->cell_sizes[b->matches[i][j]];
    if (w->neighborcount > maxM)
        maxM = w->neighborcount;
    w->mpadding = nearest_mult_256(w->neighborcount) - w->neighborcount;
    w->M = (cl_float4 *)calloc(nearest_mult_256(w->neighborcount), sizeof(cl_float4));
    int offset = 0;
    for (int j = 0; j < b->matches_counts[i]; j++)
    {
        memcpy(w->M + offset, b->cells[b->matches[i][j]], b->cell_sizes[b->matches[i][j]] * sizeof(cl_float4));
        offset += b->cell_sizes[b->matches[i][j]];
    }
    b->index++;
    if (b->index == b->idcount)
        printf("largest neighborhood was %d particles\n", maxM);
    return (w);
}