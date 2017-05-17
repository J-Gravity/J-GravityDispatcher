/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   all_work_units_done.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/16 21:48:12 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/16 21:48:14 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	all_work_units_done(t_dispatcher *dispatcher)
{
	// Output tick to file
	save_output(dispatcher, dispatcher->name);
	// Clear Work Units
	clear_work_units(dispatcher->work_units);
	// Reset work units done
	dispatcher->work_units_done = 0;
	// re-Divide the dataset into work units
	divide_dataset(dispatcher, dispatcher->dataset, &dispatcher->work_units);
	// Inform all workers work units are ready
	broadcast_worker_msg(dispatcher->workers, new_message(WORK_UNITS_READY, 0, ""));
}