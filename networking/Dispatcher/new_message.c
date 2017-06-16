/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_message.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 21:27:02 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/27 22:36:58 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_msg	new_message(char id, size_t data_size, char *data)
{
	t_msg	message;

	//printf("A#\n");
	bzero(&message, sizeof(t_msg));
	//printf("B#\n");
	message.id = id;
	//printf("C#\n");
	message.size = data_size;
	//printf("D#\n");
	message.data = (char *)calloc(1, data_size);
	//printf("E#\n");
	memcpy(message.data, data, data_size);
	//printf("F#\n");
	return (message);
}
