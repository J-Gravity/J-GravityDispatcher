/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clear_work_units.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/17 21:15:38 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/17 21:27:06 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	clear_unit(t_lst **work_units)
{	
	if (work_units == 0 || *work_units == 0)
		return ;
	if (((t_workunit *)((*work_units)->data))->local_bodies)
	{
		free(((t_workunit *)((*work_units)->data))->local_bodies);
		((t_workunit *)((*work_units)->data))->local_bodies = 0;
	}
	if (((t_workunit *)((*work_units)->data))->neighborhood)
	{
		free(((t_workunit *)((*work_units)->data))->neighborhood);
		((t_workunit *)((*work_units)->data))->neighborhood = 0;
	}
	if ((*work_units)->data)
	{
		free((*work_units)->data);
		(*work_units)->data = 0;
	}
	if ((*work_units)->next)
	{
		(*work_units)->next = 0;
	}
	if (*work_units)
	{
		free(*work_units);
		*work_units = 0;
	}
}

void		clear_work_units(t_lst **work_units)
{
	t_lst	*tmp;
	t_lst	*lst;
	
	if (work_units == 0)
		return ;
	lst = *work_units;
	while(lst)
	{
		tmp = lst->next;
		clear_unit(&lst);
		lst = tmp;
	}
	*work_units = 0;
}
