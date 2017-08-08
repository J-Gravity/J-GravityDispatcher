//toy tester to understand multi-device contexts,etc
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "worker.h"
#include "err_code.h"

typedef struct s_multicontext
{
    cl_device_id *ids;
    cl_uint device_count;
    cl_context context;
    cl_command_queue *cq;
}               t_multicontext;

static t_multicontext *setup_multicontext(void)
{
     cl_uint numPlatforms;
     t_multicontext *c = (t_multicontext *)calloc(1, sizeof(t_multicontext));
     int err;

    // Find number of platforms
    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    printf("found %d platforms\n", numPlatforms);

    // Get all platforms
    cl_platform_id Platform[numPlatforms];
    err = clGetPlatformIDs(numPlatforms, Platform, NULL);
    checkError(err, "Getting platforms");

    //how many GPUs for this platform?
    clGetDeviceIDs(Platform[0], CL_DEVICE_TYPE_GPU, 0, NULL, &(c->device_count));
    printf("%d GPUs in this platform\n", c->device_count);

    //get all those IDs
    c->ids = calloc(c->device_count, sizeof(cl_device_id));
    clGetDeviceIDs(Platform[0], CL_DEVICE_TYPE_GPU, c->device_count, c->ids, NULL);

    // // Create a compute context
    c->context = clCreateContext(0, c->device_count, c->ids, NULL, NULL, &err);
    checkError(err, "Creating context");

    // Create the command queues
    c->cq = calloc(c->device_count, sizeof(cl_command_queue));
    for (int i = 0; i < c->device_count; i++)
    	c->cq[i] = clCreateCommandQueue(c->context, c->ids[i], CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, NULL);
    printf("setup context\n");
    return (c);
}

int main(void)
{
	setup_multicontext();
}