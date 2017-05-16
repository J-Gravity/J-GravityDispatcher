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

void  connect_workers(t_dispatcher *dispatcher, t_lst **workers)
{
  int             worker_fd;

  /*
  *   TODO : Connect dispatcher to workers via TCP;
  *   : add worker to linked list;
  *   : update worker socket;
  *   : repeat for next worker;
  */

  while (*workers)
  {
    if (0 > (*workers.socket.fd = accept(dispatcher.server_sock.fd,
      &dispatcher.server_sock.addr, dispatcher.server_sock.addrlen))
    {
      if (EINTR == errno)
        continue ;
      ft_error();
    }
    //add worker to linked list
    dispatcher->workers = **workers;
    *workers = workers->next;
  }
  return ;
}
