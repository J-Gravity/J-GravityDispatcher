/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dispatcher.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/05 19:43:37 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/05 02:36:41 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DISPATCHER_H
# define DISPATCHER_H
# define _GNU_SOURCE
# define PORT 4243
# define HEADER_SIZE 5

# define BROADCAST_SUPER_PARTICLE 1
# define CACHE_REACHED_THREASHOLD 2
# define WORK_UNIT_REQUEST 3
# define ACKNOWLEDGED 42
# define WORKER_CONNECT 3
# define WORK_UNITS_READY 4
# define WORK_UNIT 6
# define WORK_UNIT_DONE 7
# define NO_WORK_UNITS 8

/* ************ */
/* TEMP         */
int G_sent_wu;

/* ************ */
/* METRIC FLAGS */
/* ************ */

# define METRICS 1
# define STARTUP_METRICS 1
# define TPM_METRIC 1
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

# define DEBUG 1
# define MSG_DEBUG 1
# define WORKER_DEBUG 0
# define MSG_DETAILS_DEBUG 0
# define MUTEX_DEBUG 0
# define DIVIDE_DATASET_DEBUG 1
# define NETWORK_DEBUG 0

# include <stdio.h>
# include <sys/socket.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <netdb.h>
# include <string.h>
# include <errno.h>
# include <CL/opencl.h>
# include <pthread.h>
# include <fcntl.h>
# include <unistd.h>
# include <signal.h>

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
	int					size;
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

typedef struct s_bounds
{
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;
}				t_bounds;

typedef struct s_cell
{
	t_body **bodies;
	int bodycount;
	struct s_cell *parent;
	struct s_cell **children;
	struct s_cell *scb;
	cl_float4 center;
	cl_float4 force_bias;
	t_bounds bounds;
}				t_cell;

typedef struct s_octree
{
	t_cell *root;
	t_body **bodies;
	size_t n_bodies;
	t_bounds bounds;
}				t_octree;

typedef struct			s_WU
{
	int					id;
	int					localcount;
	int					neighborcount;
	t_body				*local_bodies;
	t_body				*neighborhood;
	cl_float4			force_bias;
}						t_WU;

typedef struct			s_workunit
{
	int					id;
	int					localcount;
	int					neighborcount;
	t_body				**local_bodies;
	t_body				**neighborhood;
	cl_float4			force_bias;
}						t_workunit;

typedef struct			s_worker
{
	char				active;
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

typedef struct			s_dispatcher
{
	pthread_mutex_t		workunits_mutex;
	pthread_mutex_t		worker_list_mutex;
	pthread_mutex_t		workunits_done_mutex;
	char				*name;
	t_lst				*workers;
	int					worker_cnt;
	t_dataset			*dataset;
	t_dataset			*new_dataset;
	int					ticks_cnt;
	int					ticks_done;
	t_queue				*workunits;
	int					total_workunits;
	int					workunits_done;
	t_cell				**cells;
	int					cell_count;
	t_socket			sin;
	char				is_connect;
	char				is_running;
}						t_dispatcher;

typedef struct			s_thread_handler
{
	t_dispatcher		*dispatcher;
	t_lst				*worker;
}						t_thread_handler;

/*
 * 	Creates a new node and returns it
 * 		@param *workunit	The workunit to be added to the node
 */
t_lst		*queue_create_new(t_workunit workunit);

/*
 * 	Pops a node off the queue
 * 		@param **queue	A queue struct that holds first, last and size
 */
t_workunit	*queue_pop(t_queue **queue);

/*
 * 	Adds a node to the end of the queue. Returns the last param.
 * 		@param **queue	A queue struct that holds first, last and size
 * 		@param *new		The new node to be added to the queue
 */
t_lst		*queue_enqueue(t_queue **queue, t_lst *new_node);

void print_cl4(cl_float4 v);

/*
*	The function that runs on the network event handler threads.
*	handles the network events for each worker.
*		@param	input	A t_thread_handler struct that contains
*						a pointer to a worker and a pointer to the dispatcher
*/
void	*handle_worker_connection(void *input);

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
t_msg	get_worker_msg(t_worker *worker);

/*
*	fill in and return the t_msg(message) struct
*		@param	id	Message identifier (e.g. 6 for workunitS_READY)
*		@param	data_size	The size of the body of the message
*		@param	data	The body of the message
*		@return	the struct initialized with the parameters
*/
t_msg	new_message(char id, int data_size, char *data);

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
void		connect_workers(t_dispatcher *dispatcher, t_lst **workers);

/*
*	Get the next dataset (t_dataset) from the web server and store it in the
*	dispatcher struct
*		@param	init_data	Pointer to the initial dataset ptr in the dispatcher
*		@param	file		The name of the input file
*							struct
*/
void		request_dataset(t_dispatcher *dispatcher, char *file);

/*
*	Divide up the dataset into workunits and store them in the
*		@optimization	Assign them a compute_class
*		@param	dispatcher	The dispatcher's main struct
*/
void		divide_dataset(t_dispatcher *dispatcher);

/*
*		Starts the simulation by informing workers that work units are
*		ready for computaiton.
*		@param	dispatcher	The dispatcher's main struct
*/
void		launch_simulation(t_dispatcher *dispatcher);

/*
*	Save the latest tick to file with the output file format
*		@param	dispatcher	The dispatcher's main struct
*		@param	name	The name for the output files
*/
void 		save_output(t_dispatcher *dispatcher, char *name);

/*
*	Send a work unit to a specified worker
*		@param	worker	worker to recieve the work unit
*		@param	workunit	non-completed work unit
*		@return	0 if the request was fullfilled. 1 otherwise
*/
int			send_workunit(t_worker *worker, t_workunit *workunit);

/*
*	Serializes the workunit struct and stores it in the message struct
*		@param	workunit	The work unit that will be stored in the msg
*		@return message struct conataining the serialized work unit
*/
t_msg		serialize_workunit(t_workunit w);

/*
*	Parse the data of a worker message and write it to t_workunit struct
*		@param	msg	The message from the worker that contains a complete
					work unit
*/
t_WU		deserialize_WU(t_msg msg);

/*
*	Handles the worker's request for a work unit to process
*		@param	dispatcher	The dispatcher's main struct
*		@param	worker	The worker that made the request
*		@param	msg	The message sent by the worker
*/
void		handle_workunit_req(t_dispatcher *dispatcher,
			t_worker *worker, t_msg	msg);

/*
*	Handles the worker's notification that it is done with its assigned
*	work unit
*		@param	dispatcher	The dispatcher's main struct
*		@param	worker	The worker that sent the message
*		@param	msg	The message sent by the worker
*/
void		handle_worker_done_msg(t_dispatcher *dispatcher,
			t_worker *worker, t_msg	msg);

/*
*	Handles the TICK_COMPLETE_EVENT
*		@param	dispatcher	The dispatcher's main struct
*/
void		all_workunits_done(t_dispatcher *dispatcher);

/*
*	Broadcast message to all workers
*		@param	dispatcher	The dispatcher's main struct
*/
void		broadcast_worker_msg(t_lst *workers, t_msg msg);

/*
*	clears the link list of work units.
*/
void		clear_work_units(t_queue **work_units);

/*
*	clears the link list of work units.
*/
void		clear_unit(t_lst **work_units);

/*
*	returns first workunit in queue without deletion from queue
*/
t_workunit	*queue_peak(t_queue **queue);

/*
*	returns the total count of items in queue
*/
int queue_count(t_queue **queue);

/*******************************************************************************
********************************************************************************
                 POTENTIAL OPTIMIZATIONS AFTER MVP COMPLETE
********************************************************************************
*******************************************************************************/
/*
*	Coalesce all cells for each tick into a full tick
*		@param	dispatcher	The dispatcher's main struct
*		@incomplete	prototype still needs to be flushed out
*/
void		coalesce_into_ticks(t_dispatcher *dispatcher);

/*
*	Broadcast a super particle to all the cells for their computaiton
*	of the next tick (over the network)
*		@param	dispatcher	The dispatcher's main struct
*		@param	body	The super particle that represents a cell
*/
void		broadcast_super_particle(t_dispatcher *dispatcher, t_body *body);

/*
*	get the cell where the specified particle belongs
*		@param	dispatcher	The dispatcher's main struct
*		@param	body	particle to send
*		@return	cell to send it to
*/
t_cell		*find_appropriate_cell(t_dispatcher *dispatcher, t_body *body);

/*
*	Send body(particle) to a new cell
*		@param	dispatcher	The dispatcher's main struct
*		@param	body	particle to send
*		@param	cell	cell to send it to
*/
void		send_particle(t_dispatcher *dispatcher, t_body *body, t_cell *cell);

/*
*	Requests All workers to dump their cache of cell ticks to central storage
*		@param	dispatcher	The dispatcher's main struct
*		@return	0 if the request was fullfilled. 1 otherwise
*/
int 		dump_all_workers_cache(t_dispatcher *dispatcher);

/*
*	Requests a worker to dump its cache of cell ticks to central storage
*		@param	dispatcher	The dispatcher's main struct
*		@param	worker	The worker that the request will be sent to
*		@return	0 if the request was fullfilled. 1 otherwise
*/
int 		request_cache_dump(t_dispatcher *dispatcher, t_worker *worker);

/*
*	Handles the request for broadacasting a super particle to all the
*	other cells in the simulation
*		@param	dispatcher	The dispatcher's main struct
*		@param	worker	The worker that made the request
*		@param	msg	The message sent by the worker
*/
void		handle_broadcast_super_particle_req(t_dispatcher *dispatcher,
			t_worker *worker, t_msg msg);

/*
*	Handles the notificaion from the worker that their storage threashold
*	is almost reached
*		@param	dispatcher	The dispatcher's main struct
*		@param	worker	The worker that sent the notification message
*		@param	msg	The message sent by the worker
*/
void		handle_cache_threshold_reached(t_dispatcher *dispatcher,
			t_worker *worker, t_msg msg);
#endif
