/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dispatcher.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmclaugh <pmclaugh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/05 19:43:37 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/18 16:44:38 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DISPATCHER_H
# define DISPATCHER_H
# define PORT 4242
# define HEADER_SIZE 5

# define BROADCAST_SUPER_PARTICLE 1
# define CACHE_REACHED_THREASHOLD 2
# define WORK_UNIT_REQUEST 3
# define ACKNOWLEDGED 42
# define WORKER_CONNECT 3
# define WORK_UNITS_READY 4
# define WORK_UNIT 6
# define WORK_UNIT_DONE 7

# include <stdio.h>
# include <sys/socket.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <netdb.h>
# include <string.h>
# include <errno.h>
# include <OpenCL/opencl.h>

typedef struct			s_lst
{
	void 				*data;
	size_t				data_size;
	struct s_lst		*next;
}						t_lst;

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
	struct s_cell *parent;
	struct s_cell **children;
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

typedef struct			s_workunit
{
	int					id;
	int					localcount;
	int					neighborcount;
	t_body				*local_bodies;
	t_body				*neighborhood;
	cl_float4			force_bias;
}						t_workunit;

typedef struct			s_worker
{
	int					*workunit_id;
	char				compute_class;
	pthread_t			*tid;
	t_socket			socket;
}						t_worker;

typedef struct			s_serial
{
	char				*data;
	int					len;
}						t_serial;

typedef struct			s_dataset
{
	long				particle_cnt;
	double				max_scale;
	t_body				*particles;
}						t_dataset;

typedef struct			s_dispatcher
{
	char				*name;
	t_lst				*workers;
	int					worker_cnt;
	t_dataset			*dataset;
	int					ticks_cnt;
	int					ticks_done;
	t_lst				*workunits;
	int					workunits_cnt;
	int					workunits_done;
	t_cell				**cells;
	int					cell_count;
	t_socket			server_sock;
	char				is_connect;
}						t_dispatcher;

typedef struct			s_thread_handler
{
	t_dispatcher		*dispatcher;
	t_lst				*worker;
}						t_thread_handler;

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
*							struct
*/
void		request_dataset(t_dataset **init_data);

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
t_msg		serialize_workunit(t_workunit *workunit);

/*
*	Parse the data of a worker message and write it to t_workunit struct
*		@param	msg	The message from the worker that contains a complete
					work unit
*/
t_workunit	deserialize_workunit(t_msg msg);

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
void		clear_work_units(t_lst **work_units);

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
