/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clftob.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 22:43:57 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/17 22:45:14 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

char	*clftob(cl_float4 star)
{
	char	*string;

	string = calloc(1, sizeof(cl_float4));
	for (unsigned int i = 0; i < sizeof(cl_float4); i++)
		string[i] = ((char *)(&star))[i];
	return string;
}
