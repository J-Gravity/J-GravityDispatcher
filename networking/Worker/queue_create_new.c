/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_create_new.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:22:08 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/03 17:07:28 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "worker.h"

t_lst	*queue_create_new(t_workunit workunit)
{
	t_lst	*node;

	node = (t_lst *)calloc(1, sizeof(t_lst));
	node->data = (t_workunit *)calloc(1, sizeof(t_workunit));
	*(t_workunit *)(node->data) = workunit;
	printf("id:%d\n", ((t_workunit *)(node->data))->id);
	printf("lc:%d\n", ((t_workunit *)(node->data))->localcount);
	node->next = NULL;
	return (node);
}
