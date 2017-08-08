//toy tester to understand multi-device contexts,etc
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "worker.h"
#include "err_code.h"

char *const_source = "static float4 pair_force(\n" \
"    float4 pi,\n" \
"    float4 pj,\n" \
"    const float softening)\n" \
"{\n" \
"    float4 r;\n" \
"    r.x = pj.x - pi.x;\n" \
"    r.y = pj.y - pi.y;\n" \
"    r.z = pj.z - pi.z;\n" \
"    r.w = copysign(1, pi.w);\n" \
"\n" \
"    float distSquare = r.x * r.x + r.y * r.y + r.z * r.z + softening;\n" \
"    float invDist = native_rsqrt(distSquare);\n" \
"    float invDistCube = invDist * invDist * invDist;\n" \
"    float s = pj.w * invDistCube * r.w;\n" \
"    return (float4){r.x * s, r.y * s, r.z * s, 0};\n" \
"}\n" \
"\n" \
"kernel void nbody(\n" \
"    __global float4* n_start,\n" \
"    __global float4* n_end,\n" \
"    __global float4* m,\n" \
"    __global float4* v_start,\n" \
"    __global float4* v_end,\n" \
"    __local float4 *cached_stars,\n" \
"    const float softening,\n" \
"    const float timestep,\n" \
"    const float G,\n" \
"    const int N,\n" \
"    const int M,\n" \
"    const int threads_per_star)\n" \
"{\n" \
"    int globalid = get_global_id(0);\n" \
"    int chunksize = get_local_size(0);\n" \
"    int localid = get_local_id(0);\n" \
"    if (localid % threads_per_star == 0)\n" \
"    {\n" \
"        cached_stars[localid] = n_start[globalid / threads_per_star];\n" \
"        cached_stars[localid + 1] = v_start[globalid / threads_per_star];\n" \
"    }\n" \
"    barrier(CLK_LOCAL_MEM_FENCE);\n" \
"    int offset = localid - localid % threads_per_star;\n" \
"    float4 pos = cached_stars[offset];\n" \
"    float4 vel = cached_stars[offset + 1];\n" \
"    float4 force = {0,0,0,0};\n" \
"    int chunk = 0;\n" \
"    barrier(CLK_LOCAL_MEM_FENCE);\n" \
"    for (int i = 0; i < M; i += chunksize, chunk++)\n" \
"    {\n" \
"        cached_stars[localid] = m[chunk * chunksize + localid];\n" \
"        barrier(CLK_LOCAL_MEM_FENCE);\n" \
"        for (int j = 0; j < chunksize / threads_per_star;)\n" \
"        {\n" \
"            force += pair_force(pos, cached_stars[offset + j++], softening);\n" \
"            force += pair_force(pos, cached_stars[offset + j++], softening);\n" \
"            force += pair_force(pos, cached_stars[offset + j++], softening);\n" \
"            force += pair_force(pos, cached_stars[offset + j++], softening);\n" \
"        }\n" \
"        barrier(CLK_LOCAL_MEM_FENCE);\n" \
"    }\n" \
"    cached_stars[localid] = force;\n" \
"    barrier(CLK_LOCAL_MEM_FENCE);\n" \
"    if (localid % threads_per_star == 0)\n" \
"    {\n" \
"        for (int i = 1; i < threads_per_star; i++)\n" \
"            force += cached_stars[localid + i];\n" \
"        vel.x += force.x * G * timestep;\n" \
"        vel.y += force.y * G * timestep;\n" \
"        vel.z += force.z * G * timestep;\n" \
"\n" \
"        pos.x += vel.x * timestep;\n" \
"        pos.y += vel.y * timestep;\n" \
"        pos.z += vel.z * timestep;\n" \
"\n" \
"\n" \
"        n_end[globalid / threads_per_star] = pos;\n" \
"        v_end[globalid / threads_per_star] = vel;\n" \
"    }\n" \
"}\n";

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

static cl_kernel   make_kernel(t_multicontext *c, char *sourcefile, char *name)
{
    cl_kernel k;
    cl_program p;
    int err;
    //char *source;

    //source = load_cl_file(sourcefile);
    p = clCreateProgramWithSource(c->context, 1, (const char **) & const_source, NULL, &err);
    checkError(err, "Creating program");

    // Build the program
    err = clBuildProgram(p, 0, NULL, NULL, NULL, NULL);

    // Create the compute kernel from the program
    k = clCreateKernel(p, name, &err);
    checkError(err, "Creating kernel");
    clReleaseProgram(p);
    printf("made kernel\n");
    //free(source);
    return (k);
}

int main(void)
{
	t_multicontext *c = setup_multicontext();
	make_kernel(c, NULL, "mykernel");
}