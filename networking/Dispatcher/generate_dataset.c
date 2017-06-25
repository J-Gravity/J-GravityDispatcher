#include "dispatcher.h"
#define SOLAR_MASS 1

enum shape{
    sphere,
    cube,
    hollow_sphere
};

void print_float4(cl_float4 v)
{
    printf("x: %f y: %f z: %f w: %f\n", v.x, v.y, v.z, v.w);
}

float rand_float(float max)
{
    //generate random float from 0..max
    float r;

    r = (float)rand() / (float)(RAND_MAX/max);
    return r;
}

double rand_double(double max)
{
    double r;

    r = (double)rand() / (double)(RAND_MAX / max);
    return r;
}

float magnitude_3(cl_float4 v)
{
    //mag function to work on a float4 but treat it as a float3
    return (sqrt(v.x*v.x + v.y*v.y + v.z*v.z));
}

cl_float4 rand_sphere(int mag)
{
    //this creates a star from a uniform distribution in the sphere with 10^mag radius centered at 0,0,0.
    //mass is increased the closer to the center it is.
    double elevation = asin(rand_float(2) - 1);
    double azimuth = 2 * CL_M_PI * rand_float(1);
    double radius = cbrt(rand_float(1)) * __exp10(mag);
    cl_float4 out = (cl_float4){radius * cos(elevation) * cos(azimuth), \
                                radius * cos(elevation) * sin(azimuth), \
                                radius * sin(elevation), \
                                SOLAR_MASS};
    out.w = out.w * __exp10(mag) / magnitude_3(out);
    // printf("mass %f\n", out.w);
    out.w = out.w * pow(-1, rand() % 2);
    // printf("mass %f\n", out.w);
    return out;
}

cl_float4 rand_vel(int mag)
{
    return (cl_float4){pow(-1, rand() % 2) * rand_float(__exp10(mag)), \
                        pow(-1, rand() % 2) * rand_float(__exp10(mag)), \
                        pow(-1, rand() % 2) * rand_float(__exp10(mag)),
                        0};
}

cl_float4 *rotational_vels(cl_float4 *positions, int count)
{
    cl_float4 *vels = (cl_float4 *) calloc(count, sizeof(cl_float4));
    for (int i = 0; i < count; i++)
    {
        cl_float4 p = positions[i];
        float orbvel = 0.3 * sqrt(G * ANCHOR_MASS / magnitude_3(p));

        if (orbvel > 3000000)
        {
            //printf("enforced speed of light\n");
            orbvel = 3000000;
        }
        vels[i] = (cl_float4){-1 * p.y, p.x, 0, 0};
        float scale = magnitude_3(vels[i]);
        vels[i].x = orbvel * vels[i].x/scale;
        vels[i].y = orbvel * vels[i].y/scale;
        //optional noise
        cl_float4 noise = rand_vel(1);
        vels[i].x += noise.x;
        vels[i].y += noise.y;
        vels[i].z += noise.z;
    }
    return (vels);
}



cl_float4 *make_rand_vels(int n, int mag)
{
    cl_float4 *stars = (cl_float4 *)calloc(n, sizeof(cl_float4));
    for (int i = 0; i < n; i ++)
    {
        stars[i] = rand_vel(mag);
    }
    return stars;
}

cl_float4 *make_stars_sphere(int n, int mag)
{
    cl_float4 *stars = (cl_float4 *) calloc(n, sizeof(cl_float4));

    int i;
    for (i = 0; i < n; i++)
        stars[i] = rand_sphere(mag);
    printf("last i was %d\n", i);
    return (stars);
}

cl_float4 center_of_mass(cl_float4 *stars, int n)
{
    cl_float4 sum;

    sum = (cl_float4){0, 0, 0, 0};
    for (int i = 0; i < n; i++)
    {
        sum.x += stars[i].x;
        sum.y += stars[i].y;
        sum.z += stars[i].z;
        sum.w += stars[i].w;
    }
    sum.x /= sum.w;
    sum.y /= sum.w;
    sum.z /= sum.w;
    return (sum);
}

void    init_to_file(t_body *bodies, long n, char *title)
{
    int fd;
    int buffsize;

    fd = open(title, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP);
    printf("migrated vectors to bodies\n");
    long scale = (long)(2 * __exp10(BIG_RADIUS));
    write(fd, &(n), sizeof(long));
    write(fd, &(scale), sizeof(long));
    if (n * sizeof(t_body) < INT_MAX)
    {
        printf("writing in one\n");
        int ret = write(fd, bodies, sizeof(t_body) * n);
        printf("ret was %d\n", ret);
    }
    else
        for (int offset = 0; offset < n; offset += pow(2, 22))
        {
            printf("writing in chunks\n");
            write(fd, bodies + offset, sizeof(t_body) * pow(2, 22));
            fsync(fd);
        }
    close(fd);
}

t_body *generate_dataset(int starcount, int big_radius, float flattening, enum shape s)
{
    srand ( time(NULL) ); //before we do anything, seed rand() with the current time

    t_body *bodies = calloc(STARCOUNT, sizeof(t_body));
    for (int i = 0; i < STARCOUNT; i++)
    {
        bodies[i].position = rand_sphere(BIG_RADIUS);
        bodies[i].velocity = rand_vel(BIG_RADIUS);
    }
    printf("stars made, writing to disk\n");
    return (bodies);
}