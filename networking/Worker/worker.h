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
#include <stdlib.h>

# define PORT 4242
# define HEADER_SIZE 9

# define BROADCAST_SUPER_PARTICLE 1
# define CACHE_REACHED_THREASHOLD 2
# define WORK_UNIT_REQUEST 3
# define ACKNOWLEDGED 42
# define WORK_UNITS_READY 4
# define WORK_UNIT 6
# define WORK_UNIT_DONE 7
# define METRICS_DONE 9
# define WORKER_SETTINGS 10
# define SETTINGS_APPLIED 11

# include <stdio.h>
# include <sys/socket.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <netdb.h>
# include <string.h>
# include <errno.h>
# include <OpenCL/opencl.h>
# include <pthread.h>
# include <semaphore.h>
#include <sys/stat.h>

#define G 1.327 * __exp10(13) //kilometers, solar masses, (km/s)^2
#define SOFTENING 10000
#define TIME_STEP 30000
#define GROUPSIZE 256

# ifndef DEVICE
# define DEVICE CL_DEVICE_TYPE_DEFAULT
# endif

/* ******* */
/* METRICS */
/* ******* */
# define METRICS 1
long G_time_waiting_for_wu;
clock_t G_start_time;
long G_total_event_time;
long G_total_send_time;
long G_total_calc_time;

/* *********** */
/* DEBUG FLAGS */
/* *********** */
# define DEBUG 0
# define MSG_DEBUG 1
# define MSG_DETAILS_DEBUG 1
# define MUTEX_DEBUG 0
# define NETWORK_DEBUG 1

typedef struct s_context
{
    cl_device_id device_id;
    cl_context context;
    cl_command_queue commands;
}               t_context;

typedef struct			s_body
{
	cl_float4			position;
	cl_float4			velocity;
}						t_body;

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

typedef struct			s_workunit
{
	int					id;
	int					localcount;
	int 				npadding;
	int					neighborcount;
	int 				mpadding;
	cl_float4 			*N;
	cl_float4 			*M;
	cl_float4 			*V;
	t_body				*local_bodies;
	char				is_last;
}						t_workunit;

typedef struct			s_msg
{
	char				id;
	size_t					size;
	char				*data;
	int					error;
}						t_msg;

typedef	struct			s_socket
{
	int					fd;
	struct sockaddr_in	*addr;
	socklen_t			*addrlen;
	pthread_mutex_t		mutex;
}						t_socket;

typedef struct			s_worker
{
	char				active;
	float				timestep;
	float				softening;
	t_queue				*todo_work;
	t_queue				*completed_work;
	t_queue 			*bundle_queue;
	pthread_t			*event_thread;
	pthread_t			*calc_thread;
	pthread_t			*sender_thread;
	pthread_t 			*debundle_thread;
	sem_t				*sender_thread_sem;
	sem_t				*calc_thread_sem;
	sem_t				*exit_sem;
	sem_t 				*ready_for_bundle;
	sem_t 				*debundle_sem;
	t_socket			socket;
}						t_worker;

typedef struct s_bundle
{
    int idcount;
    int *ids;
    int *matches_counts;
    int **matches;
    int *local_counts;
    t_body **locals;
    int *cell_sizes;
    cl_float4 **cells;
    int cellcount;
    int index;
}               t_bundle;

void		send_metrics(int fd, t_msg msg);
t_msg		serialize_metrics(void);
t_bundle *deserialize_bundle(t_msg m);
void transpose_matches(t_bundle *wb);
t_workunit *kick_bundle(t_bundle *b);

/*
 * 	Creates a new node and returns it
 * 		@param *workunit	The workunit to be added to the node
 */
t_lst		*queue_create_new(void *workunit);

/*
 * 	Pops a node off the queue
 * 		@param **queue	A queue struct that holds first, last and size
 */
void	*queue_pop(t_queue **queue);

/*
 * 	Adds a node to the end of the queue. Returns the last param.
 * 		@param **queue	A queue struct that holds first, last and size
 * 		@param *new		The new node to be added to the queue
 */
t_lst		*queue_enqueue(t_queue **queue, t_lst *new_node);

/*
 *	Revieve a t_msg from an active tcp connection
 *		@param fd file descriptor for the open tcp connection
 */
t_msg		receive_msg(int fd);

/*
 *	Send a t_msg on an active tcp connection
 *		@param fd file descriptor for the open tcp connection
 */
void		send_msg(int fd, t_msg msg);

/*
 *	Starts listening for and handling network events
 *		@param worker The worker's master struct
 */
void		launch_event_thread(t_worker *worker);

/*
 *	Start computing workunits as they become availible
 *		@param worker The worker's master struct
 */
void		launch_calculation_thread(t_worker *worker);

/*
 *	Start sending completed workunits as they become availible
 *		@param worker The worker's master struct
 */
void		launch_sender_thread(t_worker *worker);

void		do_workunit(t_workunit *w);
t_workunit	deserialize_workunit(t_msg msg);
t_msg		serialize_workunit(t_workunit w);
void		configure_simulation(t_worker *worker, t_msg msg);

void print_cl4(cl_float4 v);

void	strbjoin(t_msg *msg, char const *s2, size_t size);
char	*itob(int value);
char	*clftob(cl_float4 star);


void launch_debundle_thread(t_worker *worker);
