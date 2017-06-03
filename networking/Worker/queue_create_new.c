/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_create_new.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:22:08 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/03 14:36:54 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

t_lst	*queue_create_new(t_workunit workunit)
{
	t_lst	*node;

	node = (t_lst *)calloc(1, sizeof(t_lst));
	*(t_workunit *)(node->data) = workunit;
	node->next = NULL;
	return (node);
}
