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
# define HEADER_SIZE 5

# define BROADCAST_SUPER_PARTICLE 1
# define CACHE_REACHED_THREASHOLD 2
# define WORK_UNIT_REQUEST 3
# define ACKNOWLEDGED 42
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

#define G 1.327 * __exp10(13) //kilometers, solar masses, (km/s)^2
#define SOFTENING 10000
#define TIME_STEP 3000000
#define THREADCOUNT pow(2, 11)
#define GROUPSIZE 256

# ifndef DEVICE
# define DEVICE CL_DEVICE_TYPE_DEFAULT
# endif

/* ******* */
/* METRICS */
/* ******* */
long G_time_waiting_for_wu;

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

typedef struct			s_workunit
{
	int					id;
	int					localcount;
	int					neighborcount;
	t_body				*local_bodies;
	cl_float4			*neighborhood;
}						t_workunit;

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
	struct sockaddr_in	*addr;
	socklen_t			*addrlen;
}						t_socket;

typedef struct			s_worker
{
	char				active;
	t_queue				workunit_queue;
	pthread_t			*event_thread;
	pthread_t			*calc_thread;
	t_socket			dispatcher_conn;
}						t_worker;


t_workunit do_workunit(t_workunit w);

t_workunit deserialize_workunit(t_msg msg);
t_msg serialize_workunit(t_workunit w);

void print_cl4(cl_float4 v);

void	strbjoin(t_msg *msg, char const *s2, size_t size);
char	*itob(int value);
char	*clftob(cl_float4 star);
