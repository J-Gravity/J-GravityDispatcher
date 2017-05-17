/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   divide_dataset.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scollet <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/09 22:43:16 by scollet           #+#    #+#             */
/*   Updated: 2017/05/17 00:08:08 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	divide_dataset(int *worker_cnt, t_dataset *dataset, t_lst **work_units)
{
	t_cell        cell;

	for (int i = 0; i < *worker_cnt; i++)
	{
		((t_work_unit *)((*work_units)->data))->cell = cell;
		((t_work_unit *)((*work_units)->data))->compute_class = 5;
    /*
    *   TODO : Recursively divide the dataset using barnes_hut();
    *   TODO : Store results of Barne's Hut Algorithm in a cell;
    *   @Optimization TODO : assign a compute_class;
    */

    /*
    *   TODO : Assign the result of Barne's Hut Algorithm to every field of the work_units;
    *     ex.: work_unit->cell->contained_bodies.position.x = barnes_hut()[0];
    */
	}
	return ;
}
