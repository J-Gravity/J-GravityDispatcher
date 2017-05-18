/*
*	clears the link list of work units.
*   free all data in individual work unit
*	move to next work unit
*
*/
#include "dispatcher.h"

void	clear_unit(t_lst **work_units)
{	
	if (work_units == 0 || *work_units == 0)
		return ;
	free(work_units->data->local_bodies);
	work_units->data->local_bodies = 0;
	free(work_units->data->neighborhood);
	work_units->data->neighborhood = 0;
	free(work_units->data);
	work_units->data = 0;
	free(*work_unit);
	*work_unit = 0;
}



void		clear_work_units(t_lst **work_units);

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
