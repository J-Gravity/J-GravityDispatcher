/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_dataset.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:41:19 by scollet           #+#    #+#             */
/*   Updated: 2017/05/18 17:39:54 by ssmith           ###   ########.fr       */
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

void  request_dataset(t_dataset **init_data)
{
	FILE  *fp;
	int   p_left;
	/*
	 *   TODO : Figure out what the hell the file will be called;
	*/
	if (NULL == (fp = fopen("./data.jgrv", "rb")))
	{
		fprintf(stderr, "Error opening file\n", errno);
		exit(0);
	}
	*init_data = (t_dataset *)calloc(1, sizeof(t_dataset));
	fread(&(*init_data)->particle_cnt, sizeof(long), 1, fp);
	fread(&(*init_data)->max_scale, sizeof(double), 1, fp);
	p_left = 1 + (*init_data)->particle_cnt;
	(*init_data)->particles = (t_body*)calloc(p_left - 1, sizeof(t_body));
	while (0 < --p_left)
	{
		if (0 > (read_into_body(fp, &(*init_data)->particles[p_left])))
		{
			fprintf(stderr, "Error reading file\n");
			exit(0);
		}
	}
	fclose(fp);
	return ;
}
