#include "dispatcher.h"

void setup_async_file(t_dispatcher *dispatcher)
{
	//makes a file of appropriate size to write the output to.
	char *filename;
	asprintf(&filename, "%s-%d.jgrav", dispatcher->name, dispatcher->ticks_done);
	pthread_mutex_lock(&dispatcher->output_mutex);
	dispatcher->fp = fopen(filename, "w");
	fwrite(&dispatcher->dataset->particle_cnt, sizeof(long), 1, dispatcher->fp);
	fseek(dispatcher->fp, sizeof(cl_float4) * dispatcher->dataset->particle_cnt, SEEK_SET);
	fputc('\0', dispatcher->fp);
	fseek(dispatcher->fp, 0, SEEK_SET);
	pthread_mutex_unlock(&dispatcher->output_mutex);
	free(filename);
}

void async_save(t_dispatcher *dispatcher, unsigned long offset, t_WU *wu)
{
	pthread_mutex_lock(&dispatcher->output_mutex);
	fseek(dispatcher->fp, offset * sizeof(cl_float4) + sizeof(long), SEEK_SET);
	for (int i = 0; i < wu->localcount; i++)
		fwrite(&wu->local_bodies[i].position, sizeof(cl_float4), 1, dispatcher->fp);
	fflush(dispatcher->fp);
	pthread_mutex_unlock(&dispatcher->output_mutex);
}