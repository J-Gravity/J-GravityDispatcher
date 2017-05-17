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

int   read_into_dataset(FILE *fp, t_dispatcher *dispatcher)
{
  int     bytes_read = 0;


  //int sscanf(const char *str, const char *format, ...)

  if (0 >= (fp))
    return (-1);
  while (bytes_read)
  {
    /* READ PARTICLE POSITION */
    fread(&dispatcher->dataset->particles->position.x,
      sizeof(float), 4, fp);
    fread(&dispatcher->dataset->particles->position.y,
      sizeof(float), 4, fp);
    fread(&dispatcher->dataset->particles->position.z,
      sizeof(float), 4, fp);
    /* READ PARTICLE VELOCITY */
    fread(&dispatcher->dataset->particles->velocity.x,
      sizeof(float), 4, fp);
    fread(&dispatcher->dataset->particles->velocity.y,
      sizeof(float), 4, fp);
    fread(&dispatcher->dataset->particles->velocity.z,
      sizeof(float), 4, fp);
    /* READ PARTICLE MASS */
    if (0 > (bytes_read = fread(&dispatcher->dataset->particles->mass,
      sizeof(float), 4, fp)))
        return (-1);

  /* PARTICLE HAS BEEN READ */
  }
  return (0);
}

void  request_dataset(t_dispatcher *dispatcher, t_dataset **init_data)
{
  FILE   *fp;

  /*
  *   TODO : Figure out what the hell the file will be;
  */

  if (0 > (fp = fopen("./data.jgrv", "r")))
  {
    fprintf(stderr, "Error opening file\n");
    exit(0);
  }
  while ((*init_data))
  {
    if (0 > (dispatcher->dataset->particle_cnt +=
      read_into_dataset(fp, dispatcher)))
    {
      fprintf(stderr, "Error reading file\n");
      exit(0);
    }
    ++init_data;
  }
  return ;
}
