/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_message.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:27:02 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/14 21:27:03 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_msg	new_message(char id, int data_size, char *data)
{
	t_msg	message;

	message.id = id;
	message.size = data_size;
	message.data = (char *)calloc(1, data_size);
	memcpy(&message.data, data, data_size);
	return (message);
}
