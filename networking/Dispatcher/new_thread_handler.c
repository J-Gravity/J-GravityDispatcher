/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_thread_handler.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/13 21:59:46 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/16 12:43:56 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

t_thread_handler	*new_thread_handler(t_dispatcher *disp, t_lst *worker)
{
	t_thread_handler	*params;

	params = (t_thread_handler	*)calloc(1, sizeof(t_thread_handler));
	params->dispatcher = disp;
	params->worker = worker;
	return (params);
}
