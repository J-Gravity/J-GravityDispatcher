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
		printf("cleared RFB semaphore\n");
		if (sem_wait(worker->debundle_sem) < 0)
		    printf("BNDL- sem_wait failed with err:%d\n", errno);
		printf("cleared debundle_sem\n");
		if (DEBUG)
			printf("BNDL- debundling\n");
		int count;
		t_bundle *bundle = queue_pop(&worker->bundle_queue);
		t_workunit **WUs = unbundle_workunits(bundle, &count);
		delete_bundle(bundle);
		for (int i = 0; i < count; i++)
		{
			queue_enqueue(&worker->todo_work, queue_create_new(WUs[i]));
			sem_post(worker->calc_thread_sem);
		}
		if (DEBUG)
			printf("BNDL- finished debundling %d workunits\n", count);
	}
	return (0);
}

void launch_debundle_thread(t_worker *worker)
{
	worker->debundle_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
	pthread_create(worker->debundle_thread, NULL, debundle_thread, worker);
}