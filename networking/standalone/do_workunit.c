#include "standalone.h"
#include "err_code.h"

static int count_bodies(t_body **bodies)
{
    int i = 0;
    if (!bodies)
        return (0);
    while (bodies[i])
        i++;
    return (i);
}

void print_cl4(cl_float4 v)
{
    printf("x: %f y: %f z: %f w:%f\n", v.x, v.y, v.z, v.w);
}

t_multicontext *setup_multicontext(void)
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
        c->cq[i] = clCreateCommandQueue(c->context, c->ids[i], 0, NULL);
    printf("setup context\n");
    return (c);
}

static char *load_cl_file(char *filename)
{
    char *source;
    int fd;

    source = (char *)calloc(1,8192);
    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        printf("could not find file\n");
        exit(1);
    }
    read(fd, source, 8192);
    return (source);
}

cl_kernel   make_kernel(t_multicontext *c, char *sourcefile, char *name)
{
    cl_kernel k;
    cl_program p;
    int err;
    char *source;

    source = load_cl_file(sourcefile);
    p = clCreateProgramWithSource(c->context, 1, (const char **) & source, NULL, &err);
    checkError(err, "Creating program");

    // Build the program
    err = clBuildProgram(p, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];

        printf("Error: Failed to build program executable!\n%s\n", err_code(err));
        clGetProgramBuildInfo(p, c->ids[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(0);
    }

    // Create the compute kernel from the program
    k = clCreateKernel(p, name, &err);
    checkError(err, "Creating kernel");
    clReleaseProgram(p);
    printf("made kernel\n");
    free(source);
    return (k);
}

size_t nearest_mult_256(size_t n)
{
    return (((n / 256) + 1) * 256);
}

// vvv N CROSS M vvv

// typedef struct          s_workunit
// {
//     int                 idx;
//     cl_float4           *N;
//     cl_float4           *V;
//     int                 n_count;
//     cl_float4           *M;
//     int                 m_count;
//     cl_event            done;
// }                       t_workunit;

void do_workunit(t_standalone *sim, t_workunit *w)
{
    int err;
    int ncount = nearest_mult_256(w->n_count);
    int mcount = nearest_mult_256(w->m_count);

    //device-side data
    cl_mem      d_N_start;
    cl_mem      d_M;
    cl_mem      d_V_start;
    cl_mem      d_V_end;
    cl_mem      d_N_end;
    //inputs
    d_N_start = clCreateBuffer(sim->context->context, CL_MEM_READ_ONLY, sizeof(cl_float4) * ncount, NULL, NULL);
    d_M = clCreateBuffer(sim->context->context, CL_MEM_READ_ONLY, sizeof(cl_float4) * mcount, NULL, NULL);
    d_V_start = clCreateBuffer(sim->context->context, CL_MEM_READ_ONLY, sizeof(cl_float4) * ncount, NULL, NULL);
    //outputs
    d_V_end = clCreateBuffer(sim->context->context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * ncount, NULL, NULL);
    d_N_end = clCreateBuffer(sim->context->context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * ncount, NULL, NULL);

    //all of that happens instantly and therefore we don't need events involved

    //copy over initial data to device locations
    cl_event loadevents[3];
    //cl_event eN, eM, eV;

    clEnqueueWriteBuffer(sim->context->cq[0], d_N_start, CL_FALSE, 0, sizeof(cl_float4) * ncount, w->N, 0, NULL, &(loadevents[0]));
    clEnqueueWriteBuffer(sim->context->cq[0], d_M, CL_FALSE, 0, sizeof(cl_float4) * mcount, w->M, 0, NULL, &(loadevents[1]));
    clEnqueueWriteBuffer(sim->context->cq[0], d_V_start, CL_FALSE, 0, sizeof(cl_float4) * ncount, w->V, 0, NULL, &(loadevents[2]));

    //printf("got past loadevents\n");
    size_t tps = 32;
    size_t global = ncount * tps;
    size_t mscale = mcount;
    size_t local = GROUPSIZE < global ? GROUPSIZE : global;
    float soften = SOFTENING;
    float timestep = TIME_STEP;
    float grav = G;
    size_t count = ncount;

    clSetKernelArg(sim->kernel, 0, sizeof(cl_mem), &d_N_start);
    clSetKernelArg(sim->kernel, 1, sizeof(cl_mem), &d_N_end);
    clSetKernelArg(sim->kernel, 2, sizeof(cl_mem), &d_M);
    clSetKernelArg(sim->kernel, 3, sizeof(cl_mem), &d_V_start);
    clSetKernelArg(sim->kernel, 4, sizeof(cl_mem), &d_V_end);
    clSetKernelArg(sim->kernel, 5, sizeof(cl_float4) * GROUPSIZE, NULL);
    clSetKernelArg(sim->kernel, 6, sizeof(float), &soften);
    clSetKernelArg(sim->kernel, 7, sizeof(float), &timestep);
    clSetKernelArg(sim->kernel, 8, sizeof(float), &grav);
    clSetKernelArg(sim->kernel, 9, sizeof(unsigned int), &global);
    clSetKernelArg(sim->kernel, 10, sizeof(unsigned int), &mscale);
    clSetKernelArg(sim->kernel, 11, sizeof(unsigned int), &tps);

    cl_event compute;

    err = clEnqueueNDRangeKernel(sim->context->cq[0], sim->kernel, 1, NULL, &global, &local, 3, loadevents, &compute);
    clEnqueueReadBuffer(sim->context->cq[0], d_N_end, CL_FALSE, 0, sizeof(cl_float4) * w->n_count, w->N, 1, &compute, &(w->done[0]));
    clEnqueueReadBuffer(sim->context->cq[0], d_V_end, CL_FALSE, 0, sizeof(cl_float4) * w->n_count, w->V, 1, &compute, &(w->done[1]));
    
    clReleaseMemObject(d_N_start);
    clReleaseMemObject(d_N_end);
    clReleaseMemObject(d_M);
    clReleaseMemObject(d_V_start);
    clReleaseMemObject(d_V_end);

    clReleaseEvent(loadevents[0]);
    clReleaseEvent(loadevents[1]);
    clReleaseEvent(loadevents[2]);
    clReleaseEvent(compute);
}