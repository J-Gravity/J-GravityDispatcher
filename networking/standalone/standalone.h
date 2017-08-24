#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#include <unistd.h>
#else
#include <CL/cl.h>
#endif
#include <time.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

typedef struct			s_lst
{
	void 				*data;
	size_t				data_size;
	struct s_lst		*next;
}						t_lst;

typedef struct			s_queue
{
	int					count;
	t_lst				*first;
	t_lst				*last;
	pthread_mutex_t		mutex;
}						t_queue;

typedef struct			s_body
{
	cl_float4			position;
	cl_float4			velocity;
}						t_body;

typedef struct			s_dataset //legacy for load_dataset, would be nice to eliminate
{
	long				particle_cnt;
	long				max_scale;
	t_body				*particles;
	cl_float4 			*positions;
	cl_float4 			*velocities;
}						t_dataset;

typedef struct			s_sortbod
{
	cl_float4 			pos;
	cl_float4 			vel;
	uint64_t			morton;
}						t_sortbod;

typedef struct			s_bounds
{
	float				xmin;
	float				xmax;
	float				ymin;
	float				ymax;
	float				zmin;
	float				zmax;
}						t_bounds;

typedef struct			s_tree
{
	//t_body				*bodies;
	cl_float4 			*positions;
	cl_float4 			*velocities;
	int					count;
	struct s_tree		**children;
	struct s_tree		*parent;
	struct s_tree		**neighbors;
	struct s_tree		*as_single;
	t_bounds			bounds;
	uint64_t			*mortons;
}						t_tree;

typedef struct 			s_workunit
{
	int 				idx;
	cl_float4 			*N;
	cl_float4 			*V;
	int 				n_count;
	cl_float4 			*M;
	int 				m_count;
	cl_event 			done[2];
}						t_workunit;

typedef struct s_multicontext
{
    cl_device_id *ids;
    cl_uint device_count;
    cl_context context;
    cl_command_queue *cq;
}               t_multicontext;

typedef struct 			s_standalone
{
	int					ticks_cnt;
	int					ticks_done;

	t_dataset			*dataset;
	t_dataset			*new_dataset;

	t_bounds 			major_bounds;
	t_tree 				**cells;
	int 				cell_count;
	int 				workunits_done;
	pthread_mutex_t 	workunits_done_mutex;

	t_queue				*todo_work;
	t_queue				*completed_work;

	pthread_t			*simulation_thread;
	pthread_t			*calc_thread;
	pthread_t			*integration_thread;

	t_multicontext 		*context;
	cl_kernel 			kernel;

	sem_t				*next_tick_sem;
	sem_t				*calc_thread_sem;
	sem_t				*integration_thread_sem;
	sem_t				*exit_sem;

	FILE 				*fp;
	pthread_mutex_t 	output_mutex;
	char 				*name;

	FILE 				*snap;
	pthread_mutex_t 	snap_mutex;
}						t_standalone;


//queue functions
t_lst	*queue_create_new(void *workunit);
void	*queue_pop(t_queue **queue);
t_lst	*queue_enqueue(t_queue **queue, t_lst *new);

//gpu stuff
#define G 1.327 * pow(10,13) //kilometers, solar masses, (km/s)^2
#define SOFTENING 10000
#define TIME_STEP 30000
#define GROUPSIZE 256
void do_workunit(t_standalone *sim, t_workunit *w);
t_multicontext *setup_multicontext(void);
cl_kernel   make_kernel(t_multicontext *c, char *sourcefile, char *name);

//thread wrappers
void		launch_simulation_thread(t_standalone *sim);
void 		launch_calculation_thread(t_standalone *sim);
void 		launch_integration_thread(t_standalone *sim);

void    	divide_dataset(t_standalone *sim);

void setup_async_file(t_standalone *sim);
void async_save(t_standalone *sim, unsigned long offset, t_workunit *wu);

void  load_dataset(t_standalone *sim, char *file);

