/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   load_dataset.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:41:19 by scollet           #+#    #+#             */
/*   Updated: 2017/06/29 00:24:33 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "standalone.h"
#include <math.h>
#include <limits.h>

// typedef struct			s_dataset
// {
// 	long				particle_cnt;
// 	long				max_scale;
// 	t_body				*particles;
//	cl_float4 			*positions;
//	cl_float4 			*velocities;
// }						t_dataset;

void transform_dataset(t_dataset *data)
{
	data->positions = (cl_float4 *)calloc(data->particle_cnt, sizeof(cl_float4));
	data->velocities = (cl_float4 *)calloc(data->particle_cnt, sizeof(cl_float4));
	for (int i = 0; i < data->particle_cnt; i++)
	{
		data->positions[i] = data->particles[i].position;
		data->velocities[i] = data->particles[i].velocity;
	}
	free(data->particles);
}

void  load_dataset(t_standalone *sim, char *file)
{
	int fd;
	long count;
	long scale;

	if ((fd = open(file, O_RDONLY)) < 1)
	{
		fprintf(stderr, "Error opening file %s\n", file, errno);
		exit(0);
	}

	
	t_dataset *data = (t_dataset *)calloc(1, sizeof(t_dataset));
	read(fd, &(count), sizeof(count));
	read(fd, &(scale), sizeof(scale));
	t_body *particles = calloc(count, sizeof(t_body));
	if (particles == NULL)
		printf("couldnt calloc that much at once\n");
	data->max_scale = scale;
	data->particle_cnt = count;

	if (count * sizeof(t_body) < INT_MAX)
    {
        printf("reading in one\n");
        int ret = read(fd, particles, sizeof(t_body) * count);
    }
    else
        for (int offset = 0; offset < count; offset += pow(2, 22))
        {
            printf("reading in chunks\n");
            int ret = read(fd, particles + offset, sizeof(t_body) * pow(2, 22));
            printf("ret was %d\n", ret);
        }
	data->particles = particles;
	transform_dataset(data);
	close(fd);
	sim->dataset = data;
	sim->new_dataset = (t_dataset *)calloc(1, sizeof(t_dataset));
	//sim->new_dataset->particles = calloc(count, sizeof(t_body));
	sim->new_dataset->positions = (cl_float4 *)calloc(sim->dataset->particle_cnt, sizeof(cl_float4));
	sim->new_dataset->velocities = (cl_float4 *)calloc(sim->dataset->particle_cnt, sizeof(cl_float4));
	sim->new_dataset->particle_cnt = sim->dataset->particle_cnt;
	sim->new_dataset->max_scale = sim->dataset->max_scale;
	return ;
}
