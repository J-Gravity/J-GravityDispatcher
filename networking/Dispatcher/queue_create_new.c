/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_create_new.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/06/02 18:22:08 by ssmith            #+#    #+#             */
/*   Updated: 2017/06/09 00:11:33 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_lst	*queue_create_new(t_bundle *bundle)
{
	t_lst	*node;

	node = (t_lst *)calloc(1, sizeof(t_lst));
	node->data = bundle;
	node->next = NULL;
	return (node);
}
