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

int   read_into_dataset(FILE *fp, t_dataset *init_data)
{
  int     bytes_read = 0;

  if (0 >= (fp))
    return (-1);
  while (bytes_read)
  {
    /* READ PARTICLE POSITION */
    fread(&init_data->particles->position.x,
      sizeof(float), 1, fp);
    fread(&init_data->particles->position.y,
      sizeof(float), 1, fp);
    fread(&init_data->particles->position.z,
      sizeof(float), 1, fp);
    /* READ PARTICLE VELOCITY */
    fread(&init_data->particles->velocity.x,
      sizeof(float), 1, fp);
    fread(&init_data->particles->velocity.y,
      sizeof(float), 1, fp);
    fread(&init_data->particles->velocity.z,
      sizeof(float), 1, fp);
    /* READ PARTICLE MASS */
    if (0 > (bytes_read = fread(&init_data->particles->mass,
      sizeof(float), 1, fp)))
        return (-1);
    /* PARTICLE HAS BEEN READ */
  }
  return (0);
}

void  request_dataset(t_dataset **init_data)
{
  FILE   *fp;

  /*
  *   TODO : Figure out what the hell the file will be called;
  */

  memset(&init_data, 0, 1);
  if (0 > (fp = fopen("./data.jgrv", "r")))
  {
    fprintf(stderr, "Error opening file\n");
    exit(0);
  }
  while ((*init_data))
  {
    *init_data = (t_dataset*)calloc(1, sizeof(t_dataset));
    if (0 > ((*init_data)->particle_cnt +=
      read_into_dataset(fp, *init_data)))
    {
      fprintf(stderr, "Error reading file\n");
      exit(0);
    }
    ++init_data;
  }
  return ;
}
