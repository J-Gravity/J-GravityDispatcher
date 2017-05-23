/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   all_workunits_done.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cyildiri <cyildiri@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/16 21:48:12 by cyildiri          #+#    #+#             */
/*   Updated: 2017/05/22 23:39:47 by cyildiri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

void	all_workunits_done(t_dispatcher *dispatcher)
{
	printf("all workunits done!\n");
	// Output tick to file
	save_output(dispatcher, dispatcher->name);
	// Clear Work Units
	//clear_work_units(&dispatcher->workunits);
	// Reset work units done
	dispatcher->workunits_done = 0;
	dispatcher->ticks_done += 1;
	// re-Divide the dataset into work units
	divide_dataset(dispatcher);
	// Inform all workers work units are ready
	broadcast_worker_msg(dispatcher->workers, new_message(WORK_UNITS_READY, 0, ""));
}
