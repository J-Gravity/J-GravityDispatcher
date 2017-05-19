/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_dataset.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:41:19 by scollet           #+#    #+#             */
/*   Updated: 2017/05/18 20:06:29 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

// typedef struct			s_dataset
// {
// 	long				particle_cnt;
// 	long				max_scale;
// 	t_body				*particles;
// }						t_dataset;

void  request_dataset(t_dataset **init_data)
{
	int fd;
	long count;
	long scale;
	/*
	 *   TODO : Figure out what the hell the file will be called;
	*/
	printf("starting request_dataset\n");
	if ((fd = open("./start.jgrav", O_RDONLY)) < 1)
	{
		fprintf(stderr, "Error opening file\n", errno);
		exit(0);
	}
	t_dataset *data = (t_dataset *)calloc(1, sizeof(t_dataset));
	read(fd, &(count), sizeof(count));
	read(fd, &(scale), sizeof(scale));
	data->particles = calloc(count, sizeof(t_body));
	data->max_scale = scale;
	data->particle_cnt = count;
	for (int i = 0; i < count; i++)
	{
		read(fd, &(data->particles[i].position), sizeof(cl_float4));
		read(fd, &(data->particles[i].velocity), sizeof(cl_float4));
	}
	close(fd);
	*init_data = data;
	printf("finished request_dataset\n");
	return ;
}
