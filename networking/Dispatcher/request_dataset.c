/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_dataset.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:41:19 by scollet           #+#    #+#             */
/*   Updated: 2017/05/23 12:31:00 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

// typedef struct			s_dataset
// {
// 	long				particle_cnt;
// 	long				max_scale;
// 	t_body				*particles;
// }						t_dataset;

void  request_dataset(t_dispatcher *dispatcher)
{
	int fd;
	long count;
	long scale;
	/*
	 *   TODO : Figure out what the hell the file will be called;
	*/
	printf("starting request_dataset\n");
	if ((fd = open("./midsize.jgrav", O_RDONLY)) < 1)
	{
		fprintf(stderr, "Error opening file\n", errno);
		exit(0);
	}
	t_dataset *data = (t_dataset *)calloc(1, sizeof(t_dataset));
	read(fd, &(count), sizeof(count));
	read(fd, &(scale), sizeof(scale));
	t_body *particles = calloc(count, sizeof(t_body));
	data->max_scale = scale;
	data->particle_cnt = count;
	read(fd, particles, sizeof(t_body) * count);
	data->particles = particles;
	close(fd);
	dispatcher->dataset = data;
	dispatcher->new_dataset = (t_dataset *)calloc(1, sizeof(t_dataset));
	dispatcher->new_dataset->particles = calloc(count, sizeof(t_body));

	//this isn't necessary in the final product, but good for making sure we're mapping correctly.
	memcpy(dispatcher->new_dataset->particles, dispatcher->dataset->particles, count * sizeof(t_body));
	printf("finished request_dataset\n");
	return ;
}
