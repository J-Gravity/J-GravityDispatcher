/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_workunit_req.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:28:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/22 18:11:45 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	handle_workunit_req(t_dispatcher *dispatcher, t_worker *worker, t_msg msg)
{
	t_lst	*head;

	printf("A$\n");
	head = dispatcher->workunits;
	printf("B$\n");
	while (head)
	{
		printf("C$\n");
		send_workunit(worker, (t_workunit *)(head->data));
		printf("D$\n");
		break ;
		printf("E$\n");
		head = head->next;
		printf("F$\n");
	}
		printf("G$\n");
}
