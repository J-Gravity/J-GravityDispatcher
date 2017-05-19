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

int   read_into_body(FILE *fp, t_body *particles)
{
  if (0 >= (fp))
    return (-1);
  /* READ PARTICLE POSITION */
  fread(&particles->position.x, sizeof(float), 1, fp);
  fread(&particles->position.y, sizeof(float), 1, fp);
  fread(&particles->position.z, sizeof(float), 1, fp);
  fread(&particles->position.w, sizeof(float), 1, fp);
  /* READ PARTICLE VELOCITY */
  fread(&particles->velocity.x, sizeof(float), 1, fp);
  fread(&particles->velocity.y, sizeof(float), 1, fp);
  fread(&particles->velocity.z, sizeof(float), 1, fp);
  fread(&particles->velocity.w, sizeof(float), 1, fp);
  /* READ PARTICLE MASS */
  return (0);
}

cl_float4 str_to_star(char *str)
{
	cl_float4 f;

	for(int i = 0; i < sizeof(cl_float4); i++)
		((char *)(&f))[i] = str[i];
	//print_float4(f);
	return f;
}

void  request_dataset(t_dataset **init_data)
{
	int fd;
	/*
	 *   TODO : Figure out what the hell the file will be called;
	*/
	printf("start request_dataset\n");
	if ((fd = open("./data.jgrv", O_RDONLY)) < 1)
	{
		fprintf(stderr, "Error opening file\n", errno);
		exit(0);
	}
	*init_data = (t_dataset *)calloc(1, sizeof(t_dataset));
	long *l;
	l = malloc(sizeof(long));
	read(fd, l, sizeof(long));
	printf("particle_cnt: %ld\n", *l);
	long pcount = *l;
	(*init_data)->particle_cnt = pcount;
	(*init_data)->particles = (t_body*)calloc(pcount, sizeof(t_body));
	read(fd, l, sizeof(long));
	printf("scale: %ld\n", *l);
	(*init_data)->max_scale = *l;
	free(l);
	char *star = malloc(sizeof(cl_float4));
	for(int i = 0; i < pcount; i++)
	{
		bzero(star, sizeof(cl_float4));
		read(fd, star, sizeof(cl_float4));
		(*init_data)->particles[i].position = str_to_star(star);
		bzero(star, sizeof(cl_float4));
		read(fd, star, sizeof(cl_float4));
		(*init_data)->particles[i].velocity = str_to_star(star);
	}
	close(fd);
	printf("finished request_dataset\n");
	return ;
}
