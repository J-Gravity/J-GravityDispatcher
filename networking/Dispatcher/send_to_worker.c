/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_to_worker.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:43:32 by scollet           #+#    #+#             */
/*   Updated: 2017/05/09 22:43:33 by scollet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

void  send_to_worker(t_dispatcher *dispatcher, t_worker worker, t_work_unit *data)
{

  /*
  *   TODO : ERROR CHECKING where needed;
  *   : define whether data is type t_work_unit or t_dataset;
  *   :
  */

  if (0 > (write(worker.socket.fd, data, data.cell.body_count)))
    ft_error();
  return ;
}
