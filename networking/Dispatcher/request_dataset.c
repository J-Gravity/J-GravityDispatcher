/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_dataset.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 20:41:19 by scollet           #+#    #+#             */
/*   Updated: 2017/05/16 13:02:30 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int   read_into_body(FILE *fp, t_body *particles)
{
  if (0 >= (fp))
    return (-1);
  /* READ PARTICLE POSITION */
  fread(&particles->position.x,
    sizeof(float), 1, fp);
  fread(&particles->position.y,
    sizeof(float), 1, fp);
  fread(&particles->position.z,
    sizeof(float), 1, fp);
  /* READ PARTICLE VELOCITY */
  fread(&particles->velocity.x,
    sizeof(float), 1, fp);
  fread(&particles->velocity.y,
    sizeof(float), 1, fp);
  fread(&particles->velocity.z,
    sizeof(float), 1, fp);
  /* READ PARTICLE MASS */
  fread(&particles->mass,
    sizeof(float), 1, fp);
  /* PARTICLE HAS BEEN READ */
  return (0);
}

void  request_dataset(t_dataset **init_data)
{
  FILE  *fp;
  int   p_left;
  /*
  *   TODO : Figure out what the hell the file will be called;
  */

  if (0 > (fp = fopen("./data.jgrv", "r")))
  {
    fprintf(stderr, "Error opening file\n");
    exit(0);
  }
  fread(&(*init_data)->particle_cnt, sizeof(int), 1, fp);
  p_left = 1 + (*init_data)->particle_cnt;
  (*init_data)->particles = (t_body*)calloc
    (p_left - 1, sizeof(t_body));
  while (0 < --p_left)
  {
    if (0 > (read_into_body(fp,
      &(*init_data)->particles[p_left])))
    {
      fprintf(stderr, "Error reading file\n");
      exit(0);
    }
  }
  fclose(fp);
  return ;
}
