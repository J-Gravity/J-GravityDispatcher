/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dispatcher.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/05 19:43:37 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/29 01:26:26 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DISPATCHER_H
# define DISPATCHER_H
# define _GNU_SOURCE
# define PORT 4342
# define HEADER_SIZE 9
# define SENDER_THREADS 32

# define LEAF_THRESHOLD pow(2, 12)
# define THETA 1
# define TIME_STEP 30000
# define SOFTENING 10000

# define BROADCAST_SUPER_PARTICLE 1
# define CACHE_REACHED_THREASHOLD 2
# define WORK_UNIT_REQUEST 3
# define ACKNOWLEDGED 42
# define WORKER_CONNECT 3
# define WORK_UNITS_READY 4
# define WORK_UNIT 6
# define WORK_UNIT_DONE 7
# define NO_WORK_UNITS 8
# define WORKER_SETTINGS 10
# define SETTINGS_APPLIED 11

/* ************ */
/* TEMP         */
/* ************ */
int G_sent_wu;

/* ************ */
/* METRIC FLAGS */
/* ************ */

# define METRICS 1
# define STARTUP_METRICS 1
# define TPM_METRIC 1
# define MUTEX_METRIC 0
# define WORKER_TIME_METRIC 0

#define TIMER_SETUP(VAR) \
	clock_t VAR_start;\
	clock_t VAR_diff;\
	int	VAR;
#define TIMER_START(VAR) \
	VAR_start = clock();
#define TIMER_STOP(VAR) \
	VAR_diff = clock() - VAR_start;\
	VAR = VAR_diff * 1000 / CLOCKS_PER_SEC;
#define TIMER_PRINT(PREFIX, VAR) \
	if (METRICS) \
		printf("%s took %d seconds %d milliseconds\n", PREFIX, VAR/1000, \
		VAR%1000);


int G_total_locked;
int G_connect_locked;
int G_movelist_locked;
int G_removeworker_locked;
int G_printfds_locked;
int G_workerevent_locked;
int G_handle_locked;
double G_tick_start;
long G_workunit_size;
long G_total_workunit_size;
double G_worker_calcs;
double G_total_time;
long G_total_workunit_cnt;
/* *********** */

/* *********** */
/* DEBUG FLAGS */
/* *********** */

# define DEBUG 0
# define MSG_DEBUG 1
# define WORKER_DEBUG 1
# define MSG_DETAILS_DEBUG 0
# define MUTEX_DEBUG 0
# define DIVIDE_DATASET_DEBUG 0
# define NETWORK_DEBUG 0

# include <stdio.h>
# include <sys/socket.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <netdb.h>
# include <string.h>
# include <errno.h>
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
# include <pthread.h>
# include <fcntl.h>
# include <unistd.h>
# include <signal.h>
# include <semaphore.h>

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

typedef struct			s_msg
{
	char				id;
	size_t				size;
	char				*data;
	int					error;
}						t_msg;

typedef	struct			s_socket
{
	int					fd;
	struct sockaddr_in	addr;
	socklen_t			addrlen;
}						t_socket;

typedef struct			s_body
{
	cl_float4			position;
	cl_float4			velocity;
}						t_body;

typedef struct			s_bounds
{
	float				xmin;
	float				xmax;
	float				ymin;
	float				ymax;
	float				zmin;
	float				zmax;
}						t_bounds;

typedef struct			s_cell
{
	t_body 				**bodies;
	int					bodycount;
	struct s_cell		*parent;
	struct s_cell		**children;
	struct s_cell		*scb;
	cl_float4			center;
	cl_float4			force_bias;
	t_bounds			bounds;
}						t_cell;

typedef struct			s_octree
{
	t_cell				*root;
	t_body				**bodies;
	size_t				n_bodies;
	t_bounds			bounds;
}						t_octree;

typedef struct			s_WU
{
	int					id;
	int					localcount;
	int					neighborcount;
	t_body				*local_bodies;
	t_body				*neighborhood;
	cl_float4			force_bias;
	char				is_last;
}						t_WU;

typedef struct			s_workunit
{
	int					id;
	int					localcount;
	int					neighborcount;
	t_body				**local_bodies;
	t_body				**neighborhood;
}						t_workunit;

typedef struct			s_worker
{
	char				active;
	int					sent_wu;
	int					rec_wu;
	t_queue				*workunit_queue;
	char				compute_class;
	pthread_t			*tid;
	t_socket			socket;
	long				w_calc_time;
}						t_worker;

typedef struct			s_serial
{
	char				*data;
	int					len;
}						t_serial;

typedef struct			s_dataset
{
	long				particle_cnt;
	long				max_scale;
	t_body				*particles;
}						t_dataset;

typedef struct			s_pair
{
	size_t				key;
	struct s_pair		*subkeys;
	struct s_pair		*next_key;
}						t_pair;

typedef struct			s_dict
{
	unsigned int		size;
	t_pair				**table;
}						t_dict;

typedef struct			s_tree
{
	t_body				*bodies;
	int					count;
	struct s_tree		**children;
	struct s_tree		*parent;
	struct s_tree		**neighbors;
	struct s_tree		*as_single;
	t_bounds			bounds;
	uint64_t			*mortons;
}						t_tree;

typedef struct			s_bundle
{
	int					*keys;
	int					keycount;
	t_tree				**cells;
	int					cellcount;
	int					**matches;
	int					*matches_counts;
	char				id;
	long				difficulty;
	long				size;
}						t_bundle;

typedef struct			s_set_data
{
	char				*set_name;
	unsigned int		star_count;
	unsigned int		solar_mass;
	unsigned int		big_radius;
	unsigned int		anchor_mass;
	unsigned int		time_step;
	unsigned int		frame_count;
	char				approved;
	char				rotating;
	unsigned int		velocity_mag;
}						t_set_data;

typedef struct			s_dispatcher
{
	pthread_mutex_t		bundles_done_mutex;
	pthread_mutex_t		sender_thread_mutex;
	pthread_t			**sender_threads;
	sem_t				*start_sending;
	sem_t				*exit_sem;
	char				*name;
	float				timestep;
	float				softening;
	t_queue				*workers;
	t_dataset			*dataset;
	t_dataset			*new_dataset;
	int					ticks_cnt;
	int					ticks_done;
	t_queue				*bundles;
	int					total_bundles;
	int					bundles_done;
	t_tree				**cells;
	int					cell_count;
	t_socket			sin;
	t_socket			cmd_sin;
	char				is_connect;
	char				is_running;
	FILE 				*fp;
	pthread_mutex_t 	output_mutex;
	t_set_data			*set_data;
}						t_dispatcher;

typedef struct			s_thread_handler
{
	t_dispatcher		*dispatcher;
	t_lst				*worker;
}						t_thread_handler;

typedef struct			s_sortbod
{
	t_body				bod;
	uint64_t			morton;
}						t_sortbod;

t_lst 			*remove_link(t_lst **list, void *data);
void			cleanup_worker(t_dispatcher *dispatcher, t_lst *worker_link);
void			configure_worker_settings(t_dispatcher *dispatcher, t_worker *worker);
t_lst			*new_lst(void *data);
t_queue			*new_queue();
t_worker		*new_worker(int fd);
t_dispatcher	*new_dispatcher(int worker_port, int cmd_port);
t_msg			serialize_settings(t_dispatcher *dispatcher);
t_set_data	*deserialize_set_data(t_msg msg);
void		receive_simulation_job(t_dispatcher *dispatcher);
void 		print_cl4(cl_float4 v);
t_body		*sort_bodies(t_body *bodies, int count);
void		tree_test(t_body *bodies, int count);
void		heap_sort(t_body *data, int n);
int			dict_search(t_dict *dict, t_tree *cell, size_t subkey);
void		dict_insert(t_dict *dict, t_tree *cell, size_t subkey);
t_dict		*create_dict(unsigned int size);
t_pair		*create_pair(size_t key);
t_bundle	*bundle_dict(t_dict *dict, t_pair *ids);
t_msg		serialize_bundle(t_bundle *b, t_tree **leaves);
void		start_sender_threads(t_dispatcher *disp, int count);
void		print_worker_fds(t_dispatcher *dispatcher);
t_lst		*queue_pop_link(t_queue **queue);
void		async_save(t_dispatcher *dispatcher, unsigned long offset, t_WU *wu);
void		setup_async_file(t_dispatcher *dispatcher);
t_body		*generate_dataset(t_set_data *sd);

/*
 * 	Creates a new node and returns it
 * 		@param *workunit	The workunit to be added to the node
 */
t_lst	*queue_create_new(void *bundle);

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
t_lst	*queue_enqueue(t_queue **queue, t_lst *new_node);

void	print_cl4(cl_float4 v);

/*
*	Create a network event thread for the provided worker
*		@param	disp	The dispatcher's main struct
*		@param	worker_link	A link of the link list of workers
*/
void	make_new_event_thread(t_dispatcher *disp, t_lst *worker_link);

/*
*	Takes an cl_float4 value and converts to a binary string
*		@value contains the cl_float4 to be converted to a string
*/

char	*clftob(cl_float4 star);

/*
*	Takes an int value and converts to a binary string
*		@value contains the int to be converted to a string
*/	

char	*itob(int value);

/*
*	Allocates and populates a string msg->data with s2
*		@msg contains current string
*		@s2 contains string to be added to end of msg->data
*		@size declares the size of s2
*/

void	strbjoin(t_msg *msg, char const *s2, size_t size);

/*
*	Allocates and populates the thread handler struct
*		@usage	can be freed safely with free();
*		@param disp	A pointer to the dispatcher's main struct
*		@param	worker	A pointer to a worker struct
*/
t_thread_handler	*new_thread_handler(t_dispatcher *disp, t_lst *worker);

/*
*	Setup a server socket endpoint
*		@param	port	the port the server will run on
*		@return	returns a struct t_socket that contains neccesary
*				data for establishing TCP connecitons
*/
t_socket	setup_server_socket(int port);

/*
*	Listen for messages from a specific worker
*		@param worker	The worker you want to listen for a message from
*/
t_msg	get_msg(int fd);

/*
*	fill in and return the t_msg(message) struct
*		@param	id	Message identifier (e.g. 6 for workunitS_READY)
*		@param	data_size	The size of the body of the message
*		@param	data	The body of the message
*		@return	the struct initialized with the parameters
*/
t_msg	new_message(char id, size_t data_size, char *data);

/*
*	Send a message to a specific worker
*		@param 	worker	The worker you want to send a message to
*		@param	msg	The message(struct) to send to the worker
*/
void	send_worker_msg(t_worker *worker, t_msg msg);

/*
*	Adds a worker to the workers linked list for each worker and populate
*	it's socket struct with the fd for the socket connection
*		@param	dispatcher	The dispatcher's main struct
*		@param	workers	A pointer to the linked list of workers
*/
void	connect_workers(t_dispatcher *dispatcher, t_lst **workers);

/*
*	Get the next dataset (t_dataset) from the web server and store it in the
*	dispatcher struct
*		@param	init_data	Pointer to the initial dataset ptr in the dispatcher
*		@param	file		The name of the input file
*							struct
*/
void	load_dataset(t_dispatcher *dispatcher, char *file);

/*
*	Divide up the dataset into workunits and store them in the
*		@optimization	Assign them a compute_class
*		@param	dispatcher	The dispatcher's main struct
*/
void	divide_dataset(t_dispatcher *dispatcher);

/*
*		Starts the simulation by informing workers that work units are
*		ready for computaiton.
*		@param	dispatcher	The dispatcher's main struct
*/
void	launch_simulation(t_dispatcher *dispatcher);

/*
*	Save the latest tick to file with the output file format
*		@param	dispatcher	The dispatcher's main struct
*		@param	name	The name for the output files
*/
void 	save_output(t_dispatcher *dispatcher, char *name);

/*
*	Send a work unit to a specified worker
*		@param	worker	worker to recieve the work unit
*		@param	workunit	non-completed work unit
*		@return	0 if the request was fullfilled. 1 otherwise
*/
int		send_workunit(t_worker *worker, t_workunit *workunit);

/*
*	Send a work unit to a specified worker
*		@param	worker	worker to recieve the work unit
*		@param	workunit	non-completed work unit
*		@return	0 if the request was fullfilled. 1 otherwise
*/
int		send_bundle(t_worker *worker, t_bundle *bundle, t_tree **leaves);

/*
*	Serializes the workunit struct and stores it in the message struct
*		@param	workunit	The work unit that will be stored in the msg
*		@return message struct conataining the serialized work unit
*/
t_msg	serialize_workunit(t_workunit w);

/*
*	Parse the data of a worker message and write it to t_workunit struct
*		@param	msg	The message from the worker that contains a complete
					work unit
*/
t_WU	deserialize_WU(t_msg msg);

/*
*	Handles the worker's request for a work unit to process
*		@param	dispatcher	The dispatcher's main struct
*		@param	worker	The worker that made the request
*		@param	msg	The message sent by the worker
*/
void	handle_workunit_req(t_dispatcher *dispatcher,
		t_worker *worker, t_msg	msg);

/*
*	Handles the worker's notification that it is done with its assigned
*	work unit
*		@param	dispatcher	The dispatcher's main struct
*		@param	worker	The worker that sent the message
*		@param	msg	The message sent by the worker
*/
void	handle_worker_done_msg(t_dispatcher *dispatcher,
		t_worker *worker, t_msg	msg);

/*
*	Handles the TICK_COMPLETE_EVENT
*		@param	dispatcher	The dispatcher's main struct
*/
void	all_bundles_done(t_dispatcher *dispatcher);

/*
*	Broadcast message to all workers
*		@param	dispatcher	The dispatcher's main struct
*/
void	broadcast_worker_msg(t_lst *workers, t_msg msg);

/*
*	clears the link list of work units.
*/
void	clear_work_units(t_queue **work_units);

/*
*	clears the link list of work units.
*/
void	clear_unit(t_lst **work_units);

/*
*	returns first workunit in queue without deletion from queue
*/
void	*queue_peek(t_queue **queue);

/*
*	returns the total count of items in queue
*/
int		queue_count(t_queue *queue);


/*
*	A multithreaded quicksort
*	takes an array and the number of elements in the array
*/
void	msort(t_sortbod *sorts, size_t count);

/*
* 	Comparison functionused for body sorting
*/

int		sbod_comp(const void *a, const void *b);


#endif
