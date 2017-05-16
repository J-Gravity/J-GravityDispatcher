/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connect_workers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 21:10:00 by scollet           #+#    #+#             */
/*   Updated: 2017/05/15 12:30:00 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <unistd.h>

void	ft_error(void)
{
	fprintf(stderr, "Value of errno: %d\n", errno);
}

void	connect_workers(t_dispatcher *dispatcher, t_lst **workers)
{
  int				worker_fd;
  pid_t				pid;
  t_dispatcher		fd;
  struct addrinfo	addr;
  t_lst				*head;

  /*
  *   TODO : Connect dispatcher to workers via TCP;
  */

  memset(&addr, 0, sizeof(addr));
  head = *workers;
  while (head)
  {
    if (0 > (worker_fd = accept(dispatcher->server_sock.fd, 0, 0)))
    {
      if (EINTR == errno)
        continue ;
      ft_error();
    }
    if (0 > (pid = fork()))
      ft_error();
    else if (0 == pid)
    {
      close(dispatcher->server_sock.fd);
      while (dispatcher->work_units->next)
        dispatcher->work_units->next = dispatcher->work_units;

    }
    else
      close(worker_fd);
	head = head->next;
  }
}
