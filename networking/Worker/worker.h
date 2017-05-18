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


#define LEAF_THRESHOLD pow(2, 14)
#define BODYCOUNT pow(2, 20)
#define BOUNDMAG 10
#define G 1.327 * __exp10(13) //kilometers, solar masses, (km/s)^2
#define SOFTENING 100000
#define TIME_STEP 1
#define THREADCOUNT pow(2, 11)
#define GROUPSIZE 256

# ifndef DEVICE
# define DEVICE CL_DEVICE_TYPE_DEFAULT
# endif

#include "err_code.h"

typedef struct s_context
{
    cl_device_id device_id;
    cl_context context;
    cl_command_queue commands;
}               t_context;

/*
	takes a workunit and computes forces on local_bodies,
	updating velocities and positions. returns a completed workunit.
*/

t_workunit *do_workunit(t_workunit *w);