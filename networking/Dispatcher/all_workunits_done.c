/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   all_workunits_done.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/16 21:48:12 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/17 22:35:30 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	all_workunits_done(t_dispatcher *dispatcher)
{
	// Output tick to file
	save_output(dispatcher, dispatcher->name);
	// Clear Work Units
	clear_work_units(&dispatcher->workunits);
	// Reset work units done
	dispatcher->workunits_done = 0;
	// re-Divide the dataset into work units
	divide_dataset(dispatcher->worker_cnt, dispatcher->dataset, &dispatcher->workunits);
	// Inform all workers work units are ready
	broadcast_worker_msg(dispatcher->workers, new_message(WORK_UNITS_READY, 0, ""));
}
