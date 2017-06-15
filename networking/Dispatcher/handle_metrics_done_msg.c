
#include "dispatcher.h"

void	handle_metrics_done_msg(t_msg msg)
{
	t_metrics	metrics;
	t_tree		*local_cell;

	metrics = deserialize_metrics(msg);
	free(msg.data);
	if (METRICS && WU_METRICS)
	{
		printf("workunit took %ld\n", metrics.total_time);
		printf("time waiting for bundle %ld\n", metrics.time_waiting_for_wu);
		printf("total event time %ld\n", metrics.total_event_time);
		printf("total send time %ld\n", metrics.total_send_time);
		printf("total calc time %ld\n", metrics.total_calc_time);
	}
}
