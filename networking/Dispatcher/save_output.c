/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   save_output.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/16 16:03:53 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/16 16:03:55 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <fcntl.h>
#include <unistd.h>

void	save_output(t_dispatcher *dispatcher, char *name)
{
	int		fd;
	char	*filename;

	asprintf(&filename, "%s-%d.jgrav", name, dispatcher->ticks_done);
	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP)) == -1)
	{
		printf("*fire* the world is burning *fire* failed to create\n"
		" the output file '%s'\n", filename);
	}
	write(fd, &dispatcher->dataset->particle_cnt, 4);
	write(fd, &dispatcher->dataset->max_scale, 4);
	write(fd, &dispatcher->dataset->particles,
			sizeof(t_body) * dispatcher->dataset->particle_cnt);
	close(fd);
}
