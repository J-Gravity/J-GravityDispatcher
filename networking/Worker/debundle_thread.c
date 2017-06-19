#include "worker.h"

static void	delete_bundle(t_bundle *bundle)
{
	if (!bundle)
		return ;
	for (int i = 0; i < bundle->idcount; i++)
	{
		free(bundle->matches[i]);
	}
	free(bundle->matches);
	free(bundle->matches_counts);
	for (int i = 0; i < bundle->cellcount; i++)
		free(bundle->cells[i]);
	free(bundle->cells);
	free(bundle->cell_sizes);
	free(bundle->ids);
	free(bundle->local_counts);
	free(bundle->locals);
	free(bundle);
}

static void *debundle_thread(void *param)
{
	t_worker	*worker;
	t_workunit  *workunit;

	worker = (t_worker *)param;
	while (1)
	{
		if (sem_wait(worker->ready_for_bundle) < 0)
			printf("BNDL- sem_wait failed with err:%d\n", errno);
		if (sem_wait(worker->debundle_sem) < 0)
		    printf("BNDL- sem_wait failed with err:%d\n", errno);
		if (DEBUG)
			printf("BNDL- debundling\n");
		t_bundle *bundle = queue_pop(&worker->bundle_queue);
		printf("this bundle has %d workunits in it\n", bundle->idcount);
		while(bundle->index < bundle->idcount)
		{
			queue_enqueue(&worker->todo_work, queue_create_new(kick_bundle(bundle)));
			sem_post(worker->calc_thread_sem);
		}
		printf("bundle is spent\n");
		delete_bundle(bundle);
		if (DEBUG)
			printf("BNDL- finished debundling %d workunits\n", bundle->idcount);
	}
	return (0);
}

void launch_debundle_thread(t_worker *worker)
{
	worker->debundle_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->debundle_thread, NULL, debundle_thread, worker);
}