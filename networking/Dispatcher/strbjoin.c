/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strbjoin.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 22:39:59 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/17 22:45:43 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	strbjoin(t_msg *msg, char const *s2, size_t size)
{
	char	*copy;
	int		i;

	i = 0;
	copy = (char *)malloc(msg->size);
	for (int x = 0; x < msg->size; x++)
		copy[x] = msg->data[x];
	msg->data = (char *)calloc(1, msg->size + size);
	while (i < msg->size)
	{
		msg->data[i] = copy[i];
		i++;
	}
	for (unsigned int x = 0; x < size; x++)
		msg->data[i + x] = s2[x];
	msg->size += size;
}
