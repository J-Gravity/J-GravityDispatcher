/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_lst.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/28 23:19:45 by cyildiri          #+#    #+#             */
/*   Updated: 2017/06/28 23:22:35 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_lst *new_lst(void *data)
{
	t_lst *new_link;

	new_link = calloc(1, sizeof(t_lst));
	new_link->data = data;
	new_link->next = NULL;
	return (new_link);
}
