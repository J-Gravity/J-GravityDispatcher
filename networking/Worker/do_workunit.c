#include "worker.h"
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


static char *load_cl_file(char *filename)
{
    char *source;
    int fd;

    source = (char *)calloc(1,8192);
    fd = open(filename, O_RDONLY);
    read(fd, source, 8192);
    return (source);
}

static t_context *setup_context(void)
{
     cl_uint numPlatforms;
     t_context *c = (t_context *)calloc(1, sizeof(t_context));
     int err;

    // Find number of platforms
    err = clGetPlatformIDs(0, NULL, &numPlatforms);

    // Get all platforms
    cl_platform_id Platform[numPlatforms];
    err = clGetPlatformIDs(numPlatforms, Platform, NULL);
    checkError(err, "Getting platforms");

    // Secure a GPU
    for (int i = 0; i < numPlatforms; i++)
    {
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
    return (c);
}

static void free_context(t_context *c)
{
    clReleaseCommandQueue(c->commands);
    clReleaseContext(c->context);
}

static cl_kernel   make_kernel(t_context *c, char *sourcefile, char *name)
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
        clGetProgramBuildInfo(p, c->device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(0);
    }

    // Create the compute kernel from the program
    k = clCreateKernel(p, name, &err);
    checkError(err, "Creating kernel");
    free(source);
    clReleaseProgram(p);
    return (k);
}

size_t nearest_mult_256(size_t n)
{
    return (((n / 256) + 1) * 256);
}

// vvv N CROSS M vvv

static t_body *crunch_NxM(cl_float4 *N, cl_float4 *V, cl_float4 *M, size_t ncount, size_t mcount, cl_float4 force_bias)
{
    srand ( time(NULL) ); //before we do anything, seed rand() with the current time
    static t_context   *context;
    static cl_kernel   k_nbody;
    int err;

    if (context == NULL)
        context = setup_context();
    if (k_nbody == NULL)
        k_nbody = make_kernel(context, "nxm.cl", "nbody");

    cl_float4 *output_p = (cl_float4 *)calloc(ncount, sizeof(cl_float4));
    cl_float4 *output_v = (cl_float4 *)calloc(ncount, sizeof(cl_float4));

    cl_float4 *FB = (cl_float4 *)calloc(ncount, sizeof(cl_float4));
    for (int i = 0; i < ncount; i++)
        FB[i] = force_bias;

    //device-side data
    cl_mem      d_N_start;
    cl_mem      d_M;
    cl_mem      d_A;
    cl_mem      d_V_start;
    cl_mem      d_V_end;
    cl_mem      d_N_end;
    //inputs
    d_N_start = clCreateBuffer(context->context, CL_MEM_READ_ONLY, sizeof(cl_float4) * ncount, NULL, NULL);
    d_M = clCreateBuffer(context->context, CL_MEM_READ_ONLY, sizeof(cl_float4) * mcount, NULL, NULL);
    d_V_start = clCreateBuffer(context->context, CL_MEM_READ_ONLY, sizeof(cl_float4) * ncount, NULL, NULL);
    //outputs
    d_A = clCreateBuffer(context->context, CL_MEM_READ_WRITE, sizeof(cl_float4) * ncount, NULL, NULL);
    d_V_end = clCreateBuffer(context->context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * ncount, NULL, NULL);
    d_N_end = clCreateBuffer(context->context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * ncount, NULL, NULL);

    //all of that happens instantly and therefore we don't need events involved

    //copy over initial data to device locations
    cl_event eN, eM, eA, eV;
    clEnqueueWriteBuffer(context->commands, d_N_start, CL_TRUE, 0, sizeof(cl_float4) * ncount, N, 0, NULL, &eN);
    clEnqueueWriteBuffer(context->commands, d_M, CL_TRUE, 0, sizeof(cl_float4) * mcount, M, 0, NULL, &eM);
    clEnqueueWriteBuffer(context->commands, d_A, CL_TRUE, 0, sizeof(cl_float4) * ncount, FB, 0, NULL, &eA);
    clEnqueueWriteBuffer(context->commands, d_V_start, CL_TRUE, 0, sizeof(cl_float4) * ncount, V, 0, NULL, &eV);

    cl_event loadevents[4] = {eN, eM, eA, eV};

    size_t global = ncount;
    size_t mscale = mcount;
    size_t local = GROUPSIZE < global ? GROUPSIZE : global;
    float soften = SOFTENING;
    float timestep = TIME_STEP;
    float grav = G;
    size_t count = ncount;

    clSetKernelArg(k_nbody, 0, sizeof(cl_mem), &d_N_start);
    clSetKernelArg(k_nbody, 1, sizeof(cl_mem), &d_N_end);
    clSetKernelArg(k_nbody, 2, sizeof(cl_mem), &d_M);
    clSetKernelArg(k_nbody, 3, sizeof(cl_mem), &d_V_start);
    clSetKernelArg(k_nbody, 4, sizeof(cl_mem), &d_V_end);
    clSetKernelArg(k_nbody, 5, sizeof(cl_mem), &d_A);
    clSetKernelArg(k_nbody, 6, sizeof(cl_float4) * GROUPSIZE, NULL);
    clSetKernelArg(k_nbody, 7, sizeof(float), &soften);
    clSetKernelArg(k_nbody, 8, sizeof(float), &timestep);
    clSetKernelArg(k_nbody, 9, sizeof(float), &grav);
    clSetKernelArg(k_nbody, 10, sizeof(unsigned int), &global);
    clSetKernelArg(k_nbody, 11, sizeof(unsigned int), &mscale);
    
    //printf("global is %zu, local is %zu\n", global, local);
    //printf("going onto the GPU\n");
    
    cl_event compute;
    cl_event offN, offV;
    err = clEnqueueNDRangeKernel(context->commands, k_nbody, 1, NULL, &global, &local, 4, loadevents, &compute);
    checkError(err, "Enqueueing kernel");
    clEnqueueReadBuffer(context->commands, d_N_end, CL_TRUE, 0, sizeof(cl_float4) * count, output_p, 1, &compute, &offN);
    clEnqueueReadBuffer(context->commands, d_V_end, CL_TRUE, 0, sizeof(cl_float4) * count, output_v, 1, &compute, &offV);
    clFinish(context->commands);

    //these will have to happen elsewhere in final but here is good for now
    clReleaseMemObject(d_N_start);
    clReleaseMemObject(d_N_end);
    clReleaseMemObject(d_M);
    clReleaseMemObject(d_V_start);
    clReleaseMemObject(d_V_end);
    clReleaseMemObject(d_A);

    clReleaseEvent(eN);
    clReleaseEvent(eM);
    clReleaseEvent(eA);
    clReleaseEvent(eV);
    clReleaseEvent(compute);
    clReleaseEvent(offN);
    clReleaseEvent(offV);

    // printf("after computation, in output buffers\n");
    // print_cl4(output_p[0]);
    // print_cl4(output_v[0]);

    t_body *ret = (t_body *)malloc(sizeof(t_body) * ncount);
    for (int i = 0; i < ncount; i++)
    {
        ret[i].position = output_p[i];
        ret[i].velocity = output_v[i];
    }
    free(output_p);
    free(output_v);
    //free_context(context);
    //clReleaseKernel(k_nbody);
    free(FB);
    return (ret);
}

t_workunit do_workunit(t_workunit w)
{
    // printf("before computation, from WU\n");
    // print_cl4(w.local_bodies[0].position);
    // print_cl4(w.local_bodies[0].velocity);
    cl_float4 fb = w.force_bias;
    size_t ncount = w.localcount;
    size_t mcount = w.neighborcount;
    size_t npadding = nearest_mult_256(ncount) - ncount;
    size_t mpadding = nearest_mult_256(mcount) - mcount;
    cl_float4 *N = (cl_float4 *)calloc(ncount + npadding, sizeof(cl_float4));
    cl_float4 *M = (cl_float4 *)calloc(mcount + mpadding, sizeof(cl_float4));
    cl_float4 *V = (cl_float4 *)calloc(ncount + npadding, sizeof(cl_float4));
    for (int i = 0; i < ncount; i++)
    {
        N[i] = w.local_bodies[i].position;
        V[i] = w.local_bodies[i].velocity;
    }
    // printf("before computation, in input buffers\n");
    // print_cl4(N[0]);
    // print_cl4(V[0]);
    for (int i = 0; i < mcount; i++)
    {
        M[i] = w.neighborhood[i].position;
    }
    free(w.local_bodies);
    w.local_bodies = crunch_NxM(N, V, M, ncount + npadding, mcount + mpadding, fb);
    free(w.neighborhood);
    w.neighborhood = NULL;
    w.neighborcount = 0;
    free(N);
    free(M);
    free(V);
    // printf("after computation, in WU\n");
    // print_cl4(w.local_bodies[0].position);
    // print_cl4(w.local_bodies[0].velocity);
    return (w);
}