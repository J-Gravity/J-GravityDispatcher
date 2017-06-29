/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_worker.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/28 23:04:42 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/28 23:20:04 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_worker	*new_worker(int fd)
{
	t_worker	*worker;

	worker = calloc(1, sizeof(t_worker));
	worker->socket.fd = fd;
	worker->tid = 0;
	worker->workunit_queue = new_queue();
	return (worker);
}
