/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   itob.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 22:44:27 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/17 22:45:28 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

char	*itob(int value)
{
	char	*string;

	string = calloc(1, sizeof(int));
	for (unsigned int i = 0; i < sizeof(int); i++)
	{
		string[i] = ((char *)(&value))[i];
		i++;
	}
	return string;	
}
