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
	if ((fd = open("./2-19.jgrav", O_RDONLY)) < 1)
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
	*init_data = data;
	printf("finished request_dataset\n");
	return ;
}
