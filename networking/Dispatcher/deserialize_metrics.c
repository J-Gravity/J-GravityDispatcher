
#include "dispatcher.h"

t_metrics	deserialize_metrics(t_msg msg)
{
	t_metrics	metrics;
	int			local_compressed_size;

	int offset = 0;
	metrics.total_time = 0;
	metrics.time_waiting_for_wu = 0;
	metrics.total_event_time = 0;
	metrics.total_calc_time = 0;
	metrics.total_send_time = 0;
	memcpy(&(metrics.id), msg.data, sizeof(int));
	offset += sizeof(int);
	memcpy(&(metrics.total_time), msg.data + offset, sizeof(long));
	offset += sizeof(long);
	memcpy(&(metrics.time_waiting_for_wu), msg.data + offset, sizeof(long));
	offset += sizeof(long);
	memcpy(&(metrics.total_event_time), msg.data + offset, sizeof(long));
	offset += sizeof(long);
	memcpy(&(metrics.total_calc_time), msg.data + offset, sizeof(long));
	offset += sizeof(long);
	memcpy(&(metrics.total_send_time), msg.data + offset, sizeof(long));
	offset += sizeof(long);
	return (metrics);
}
