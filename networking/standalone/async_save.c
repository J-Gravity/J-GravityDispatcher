#include "standalone.h"

void setup_async_file(t_standalone *sim)
{
	//makes a file of appropriate size to write the output to.
	char *filename;
	asprintf(&filename, "%s-%d.jgrav", sim->name, sim->ticks_done);
	pthread_mutex_lock(&sim->output_mutex);
	sim->fp = fopen(filename, "w");
	fwrite(&sim->dataset->particle_cnt, sizeof(long), 1, sim->fp);
	fseek(sim->fp, sizeof(cl_float4) * sim->dataset->particle_cnt, SEEK_SET);
	fputc('\0', sim->fp);
	fseek(sim->fp, 0, SEEK_SET);
	pthread_mutex_unlock(&sim->output_mutex);
	free(filename);
}

void setup_snap_file(t_standalone *sim)
{
	char *filename;
	asprintf(&filename, "%s-snap-%d.jgrav", sim->name, sim->ticks_done);
	pthread_mutex_lock(&sim->snap_mutex);
	sim->snap = fopen(filename, "w");
	fwrite(&sim->cell_count, sizeof(long), 1, sim->snap);
	// fseek(sim->snap, sizeof(cl_float4) * sim->cell_count, SEEK_SET);
	// fputc('\0', sim->snap);
	// fseek(sim->snap, 0, SEEK_SET);
	pthread_mutex_unlock(&sim->snap_mutex);
	free(filename);
}

void snap_save(t_standalone *sim)
{
	setup_snap_file(sim);
	printf("file set up\n");
	pthread_mutex_lock(&sim->snap_mutex);
	//simple single threaded while i think
	for(int i = 0; i < sim->cell_count; i++)
		fwrite(sim->cells[i]->as_single->positions, sizeof(cl_float4), 1, sim->snap);
	fflush(sim->snap);
	//this is probaby fine. multithreaded is easy because order doesn't matter just mutex the writes
	//when calculating cog_from_bodies, after that just write self
	pthread_mutex_unlock(&sim->snap_mutex);
}

void async_save(t_standalone *sim, unsigned long offset, t_workunit *wu)
{
	pthread_mutex_lock(&sim->output_mutex);
	fseek(sim->fp, offset * sizeof(cl_float4) + sizeof(long), SEEK_SET);
	fwrite(wu->N, sizeof(cl_float4), wu->n_count, sim->fp);
	pthread_mutex_unlock(&sim->output_mutex);
}