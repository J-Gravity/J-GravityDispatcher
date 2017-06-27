/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_dataset.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:41:19 by scollet           #+#    #+#             */
/*   Updated: 2017/05/30 23:34:33 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <math.h>
#include <limits.h>

// typedef struct			s_dataset
// {
// 	long				particle_cnt;
// 	long				max_scale;
// 	t_body				*particles;
// }						t_dataset;

void  request_dataset(t_dispatcher *dispatcher, char *file)
{
	int fd;
	long count;
	long scale;
	/*
	 *   TODO : Figure out what the hell the file will be called;
	*/

	if (dispatcher->set_data)
	{
		dispatcher->dataset = (t_dataset *)calloc(1, sizeof(t_dataset));
		dispatcher->dataset->particles = generate_dataset(dispatcher->set_data);
		dispatcher->dataset->particle_cnt = dispatcher->set_data->star_count;

		dispatcher->new_dataset = (t_dataset *)calloc(1, sizeof(t_dataset));
		dispatcher->new_dataset->particles = calloc(dispatcher->set_data->star_count, sizeof(t_body));
		dispatcher->new_dataset->particle_cnt = dispatcher->dataset->particle_cnt;

		return ;
	}

	if (DEBUG)
		printf("starting request_dataset\n");
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
	close(fd);
	dispatcher->dataset = data;
	dispatcher->new_dataset = (t_dataset *)calloc(1, sizeof(t_dataset));
	dispatcher->new_dataset->particles = calloc(count, sizeof(t_body));
	dispatcher->new_dataset->particle_cnt = dispatcher->dataset->particle_cnt;
	dispatcher->new_dataset->max_scale = dispatcher->dataset->max_scale;
	if (DEBUG)
		printf("finished request_dataset\n");
	return ;
}
