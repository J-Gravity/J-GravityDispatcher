/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   save_output.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/16 16:03:53 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/30 14:32:16 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <fcntl.h>
#include <unistd.h>

void	save_output(t_dispatcher *dispatcher, char *name)
{
	int		fd;
	char	*filename;
	char *bigbuff;

	asprintf(&filename, "%s-%d.jgrav", name, dispatcher->ticks_done);

	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP)) == -1)
	{
		printf("*fire* the world is burning *fire* failed to create\n"
		" the output file '%s'\n", filename);
	}
	write(fd, &dispatcher->dataset->particle_cnt, sizeof(long));
	//write(fd, &dispatcher->dataset->max_scale, sizeof(long));
	bigbuff = (char *)calloc(dispatcher->dataset->particle_cnt, sizeof(cl_float4));
	for(int i = 0; i < dispatcher->dataset->particle_cnt; i++)
		memcpy(bigbuff + i * sizeof(cl_float4), &dispatcher->dataset->particles[i].position, sizeof(cl_float4));
	write(fd, bigbuff, dispatcher->dataset->particle_cnt * sizeof(cl_float4));
	free(bigbuff);
	free(filename);
	close(fd);
}
