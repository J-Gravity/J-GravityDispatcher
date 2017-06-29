/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   remove_link.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/29 00:43:55 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/29 00:44:32 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_lst 		*remove_link(t_lst **list, void *data)
{
	t_lst	*head;
	t_lst	*last;

	head = *list;
	last = *list;
	if (head && head->data == data)
	{
		*list = head->next;
		return (head);
	}
	while (head)
	{
		if (head->data == data)
		{
			last->next = head->next;
			return (head);
		}
		last = head;
		head = head->next;
	}
	return (NULL);
}
