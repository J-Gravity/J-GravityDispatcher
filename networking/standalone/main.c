#include "standalone.h"

static void unlink_semaphores()
{
    sem_unlink("/next_tick");
	sem_unlink("/calc_thread");
	sem_unlink("/integration_thread");
	sem_unlink("/exit");
}

static void initialize_semaphores(t_standalone *sim)
{
	unlink_semaphores();
	sim->next_tick_sem = sem_open("/next_tick", O_CREAT, 077, 0);
	if (sim->next_tick_sem == SEM_FAILED)
		printf("sem4 open failed with %d\n", errno);
	sim->calc_thread_sem = sem_open("/calc_thread", O_CREAT, 0777, 0);
	if (sim->calc_thread_sem == SEM_FAILED)
		printf("sem1 open failed with %d\n", errno);
	sim->integration_thread_sem = sem_open("/integration_thread", O_CREAT, 0777, 0);
	if (sim->integration_thread_sem == SEM_FAILED)
		printf("sem2 open failed with %d\n", errno);
	sim->exit_sem = sem_open("/exit", O_CREAT, 0777, 0);
	if (sim->exit_sem == SEM_FAILED)
		printf("sem3 open failed with %d\n", errno);
}

static void initialize_queues(t_standalone *sim)
{

	t_queue				*todo_work;
	t_queue				*completed_work;

	sim->todo_work = (t_queue *)calloc(1, sizeof(t_queue));
	sim->completed_work = (t_queue *)calloc(1, sizeof(t_queue));
	pthread_mutex_init(&sim->todo_work->mutex, NULL);
	pthread_mutex_init(&sim->completed_work->mutex, NULL);
	pthread_mutex_init(&sim->workunits_done_mutex, NULL);
	pthread_mutex_init(&sim->output_mutex, NULL);
	pthread_mutex_init(&sim->snap_mutex, NULL);
}

int main(int ac, char **av)
{
	t_standalone *sim = calloc(1, sizeof(t_standalone));
	//need some setup code here like ticks_cnt
	sim->ticks_cnt = 1800;
	sim->name = strdup("local");
	
	sim->context = setup_multicontext();
	sim->kernel = make_kernel(sim->context, "nxm2.cl", "nbody");

	if (ac == 1)
	{
		printf("Usage ./a.out <inital_dataset.jgrav file>\n");
		exit(1);
	}

	//load inital_dataset
	load_dataset(sim, av[1]); //converted to t_standalone

	initialize_semaphores(sim);
	initialize_queues(sim);

	launch_simulation_thread(sim);//divide_dataset thread, puts shell WUs on q
	launch_calculation_thread(sim);//serialize_workunit, do_workunit
	launch_integration_thread(sim);//integrate, async save

	int val = sem_wait(sim->exit_sem);
	unlink_semaphores();
	return (0);
}

/*
	-radically improve sorting
		-integration of sortbods too
	-make video of good-sized janus run
	-start fucking with the renderer i guess
	-simulation thread would benefit a LOT from not making and ending threads all the time
*/