//toy tester to understand multi-device contexts,etc

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

    // // Create a command queue
    // c->commands = clCreateCommandQueue(c->context, (c->device_id), 0, &err);
    // checkError(err, "Creating command queue");
    // printf("setup context\n");
    return (c);
}

int main(void)
{
	setup_multicontext();
}
