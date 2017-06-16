static float4 pair_force(
    float4 pi,
    float4 pj,
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
    return (float4){r.x * s, r.y * s, r.z * s, 0};
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
    const int M,
    const int threads_per_star)
{
    int globalid = get_global_id(0);
    int chunksize = get_local_size(0);
    int localid = get_local_id(0);
    if (localid % threads_per_star == 0)
    {
        cached_stars[localid] = n_start[globalid / threads_per_star];
        cached_stars[localid + 1] = v_start[globalid / threads_per_star];
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    int offset = localid - localid % threads_per_star;
    float4 pos = cached_stars[offset];
    float4 vel = cached_stars[offset + 1];
    float4 force = {0,0,0,0};
    int chunk = 0;
    barrier(CLK_LOCAL_MEM_FENCE);
    for (int i = 0; i < M; i += chunksize, chunk++)
    {
        cached_stars[localid] = m[chunk * chunksize + localid];
        barrier(CLK_LOCAL_MEM_FENCE);
        for (int j = 0; j < chunksize / threads_per_star;)
        {
            force += pair_force(pos, cached_stars[offset + j++], softening);
            force += pair_force(pos, cached_stars[offset + j++], softening);
            force += pair_force(pos, cached_stars[offset + j++], softening);
            force += pair_force(pos, cached_stars[offset + j++], softening);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    cached_stars[localid] = force;
    barrier(CLK_LOCAL_MEM_FENCE);
    if (localid % threads_per_star == 0)
    {
        for (int i = 1; i < threads_per_star; i++)
            force += cached_stars[localid + i];

        vel.x += force.x * G * timestep;
        vel.y += force.y * G * timestep;
        vel.z += force.z * G * timestep;

        pos.x += vel.x * timestep;
        pos.y += vel.y * timestep;
        pos.z += vel.z * timestep;

        n_end[globalid / threads_per_star] = pos;
        v_end[globalid / threads_per_star] = vel;
    }
}