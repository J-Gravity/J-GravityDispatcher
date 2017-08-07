//toy tester to understand multi-device contexts,etc

#include "worker.h"
#include "err_code.h"

typedef struct s_multicontext
{
    cl_device_id *ids;
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
    cl_uint numGPUs;
    clGetDeviceIDs(Platform[0], CL_DEVICE_TYPE_GPU, 0, NULL, &numGPUs);
    printf("%d GPUs in this platform\n");

    // Secure a GPU
    for (int i = 0; i < numPlatforms; i++)
    {
    	char *platName = malloc(1024);
    	clGetPlatformInfo(Platform[i], CL_PLATFORM_NAME, 1024, platName, NULL);
    	printf("%s platform\n", platName);
        err = clGetDeviceIDs(Platform[i], DEVICE, 1, &(c->device_id), NULL);
        if (err == CL_SUCCESS)
        {
            break;
        }
    }

    if (c->device_id == NULL)
        checkError(err, "Finding a device");

    // Create a compute context
    c->context = clCreateContext(0, 1, &(c->device_id), NULL, NULL, &err);
    checkError(err, "Creating context");

    // Create a command queue
    c->commands = clCreateCommandQueue(c->context, (c->device_id), 0, &err);
    checkError(err, "Creating command queue");
    printf("setup context\n");
    return (c);
}

int main(void)
{
	setup_context();
}