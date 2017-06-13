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
	if (DEBUG)
		printf("starting request_dataset\n");
	if ((fd = open(file, O_RDONLY)) < 1)
	{
		fprintf(stderr, "Error opening file %s\n", file, errno);
		exit(0);
	}
	t_dataset *data = (t_dataset *)calloc(1, sizeof(t_dataset));
	read(fd, &(count), sizeof(long));
	read(fd, &(scale), sizeof(long));
	t_body *particles = calloc(count, sizeof(t_body));
	data->max_scale = scale;
	data->particle_cnt = count;
	read(fd, particles, sizeof(t_body) * count);
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
