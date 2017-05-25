static float4 pair_force(
    float4 pi,
    float4 pj,
    float4 ai,
    const float softening)
{
    float4 r;
    r.x = pj.x - pi.x;
    r.y = pj.y - pi.y;
    r.z = pj.z - pi.z;
    r.w = copysign(1, pi.w);

    float distSquare = r.x * r.x + r.y * r.y + r.z * r.z + softening;
    float invDist = native_rsqrt(distSquare);
    float invDistCube = invDist * invDist * invDist;
    float s = pj.w * invDistCube * r.w;
    ai.x += r.x * s;
    ai.y += r.y * s;
    ai.z += r.z * s;
    return ai;
}

kernel void nbody(
    __global float4* n_start,
    __global float4* n_end,
    __global float4* m,
    __global float4* v_start,
    __global float4* v_end,
    __local float4 *cached_stars,
    const float softening,
    const float timestep,
    const float G,
    const int N,
    const int M)
{
    int globalid = get_global_id(0);
    int threadcount = get_global_size(0);
    int chunksize = get_local_size(0);
    int localid = get_local_id(0);
    
    float4 pos = n_start[globalid];
    float4 vel = v_start[globalid];
    float4 force = {0,0,0,0};

    int chunk = 0;
    for (int i = 0; i < M; i += chunksize, chunk++)
    {
        int local_pos = chunk * chunksize + localid;
        cached_stars[localid] = m[local_pos];

        barrier(CLK_LOCAL_MEM_FENCE);
        for (int j = 0; j < chunksize;)
        {
            force = pair_force(pos, cached_stars[j++], force, softening);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    vel.x += force.x * G * timestep;
    vel.y += force.y * G * timestep;
    vel.z += force.z * G * timestep;

    pos.x += vel.x * timestep;
    pos.y += vel.y * timestep;
    pos.z += vel.z * timestep;

    n_end[globalid] = pos;
    v_end[globalid] = vel;
}