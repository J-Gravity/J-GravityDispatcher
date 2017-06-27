#include "dispatcher.h"
#include <math.h>

// typedef struct      s_set_data
// {
//     char            *set_name;
//     unsigned int    star_count;
//     unsigned int    solar_mass;
//     unsigned int    big_radius;
//     unsigned int    anchor_mass;
//     unsigned int    time_step;
//     unsigned int    frame_count;
//     char            approved;
// }                   t_set_data;

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
    double radius = cbrt(rand_float(1)) * pow(10, mag);
    cl_float4 out = (cl_float4){radius * cos(elevation) * cos(azimuth), \
                                radius * cos(elevation) * sin(azimuth), \
                                radius * sin(elevation), \
                                1};
    out.w = out.w * pow(10, mag) / magnitude_3(out);
    // printf("mass %f\n", out.w);
    out.w = out.w * pow(-1, rand() % 2);
    // printf("mass %f\n", out.w);
    return out;
}

cl_float4 rand_vel(int mag)
{
    return (cl_float4){pow(-1, rand() % 2) * rand_float(pow(10, mag)), \
                        pow(-1, rand() % 2) * rand_float(pow(10, mag)), \
                        pow(-1, rand() % 2) * rand_float(pow(10, mag)),
                        0};
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

// typedef struct      s_set_data
// {
//     char            *set_name;
//     unsigned int    star_count;
//     unsigned int    solar_mass;
//     unsigned int    big_radius;
//     unsigned int    anchor_mass;
//     unsigned int    time_step;
//     unsigned int    frame_count;
//     char            approved;
// }                   t_set_data;

t_body *generate_dataset(t_set_data *sd)
{
    srand ( time(NULL) ); //before we do anything, seed rand() with the current time

    t_body *bodies = calloc(sd->star_count, sizeof(t_body));
    for (int i = 0; i < sd->star_count; i++)
    {
        bodies[i].position = rand_sphere(sd->big_radius);
        bodies[i].velocity = rand_vel(sd->big_radius / 2);
    }
    if (sd->anchor_mass && sd->star_count)
    {
        bodies[0].position = (cl_float4){0,0,0,sd->anchor_mass};
        bodies[0].velocity = (cl_float4){0, 0, 0, 0};
    }
    return (bodies);
}