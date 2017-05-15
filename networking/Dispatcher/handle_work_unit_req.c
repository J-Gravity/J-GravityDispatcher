/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_work_unit_req.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/14 21:49:57 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

void	handle_work_unit_req(t_dispatcher *dispatcher, t_worker *worker, t_msg msg)
{
	t_lst	*head;

	head = dispatcher->work_units;
	while (head)
	{
		if ((t_work_unit *)(head->data)->complete == 0)
		{
			(t_work_unit *)(head->data)->complete = 1;
			send_work_unit(worker, (t_work_unit *)head);
			break ;
		}
		head = head->next;
	}
}
