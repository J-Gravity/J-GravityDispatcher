//Created by myoung

#include "dispatcher.h"

t_set_data *deserialize_set_data(t_msg msg)
{
	t_set_data *set_data;
	unsigned int offset;
	unsigned int set_name_len;

	offset = 0;
	set_data = calloc(1, sizeof(t_set_data*));
	memcpy(&(set_name_len), msg.data + offset, sizeof(unsigned int));
	offset += sizeof(unsigned int); //offset to set_name
	printf("set_name_len: %d\n", set_name_len);
	set_data->set_name = (char*)malloc(set_name_len);
	memcpy(set_data->set_name, msg.data + offset, set_name_len);
	offset += set_name_len; //offset to star_count
	memcpy(&(set_data->star_count), msg.data + offset, sizeof(unsigned int));
	offset += sizeof(unsigned int); //offset to solar_mass
	memcpy(&(set_data->solar_mass), msg.data + offset, sizeof(unsigned int));
	offset += sizeof(unsigned int); //offset to big_radius
	memcpy(&(set_data->big_radius), msg.data + offset, sizeof(unsigned int));
	offset += sizeof(unsigned int); //offset to anchor_mass
	memcpy(&(set_data->anchor_mass), msg.data + offset, sizeof(unsigned int));
	offset += sizeof(unsigned int); //offset to time_step
	memcpy(&(set_data->time_step), msg.data + offset, sizeof(unsigned int));
	offset += sizeof(unsigned int); //offset to frame_count
	memcpy(&(set_data->frame_count), msg.data + offset, sizeof(unsigned int));
	offset += sizeof(unsigned int); //offset to approved
	memcpy(&(set_data->approved), msg.data + offset, sizeof(char));
	offset += sizeof(char); //offset to approved
	return (set_data);
}

void receive_simulation_job(t_dispatcher *dispatcher)
{
	int			fd;
	t_msg		msg;
	t_set_data	*set_data;

	printf("%s\n", "Waiting on the signal from jgrav-host");
	fd = accept(dispatcher->sin.fd, (struct sockaddr *)&(dispatcher->sin.addr.sin_addr), &(dispatcher->sin.addrlen));
	printf("received jgrav-host message\n");
	if (fd == 0)
		printf("jgrav-host accept returned 0");
	else if (fd == -1)
		printf("jgrav-host accept failed with %d\n", errno);
	else
	{
		msg = get_msg(fd);
		printf("message id: %c\n", msg.id);
		printf("message size: %zu\n", msg.size);
		printf("message data: %s\n", msg.data);
		set_data = deserialize_set_data(msg);
		printf("set_data->set_name: %s\n", set_data->set_name);
		printf("set_data->star_count: %d\n", set_data->star_count);
		printf("set_data->big_radius: %d\n", set_data->big_radius);
		printf("set_data->anchor_mass: %d\n", set_data->anchor_mass);
		printf("set_data->time_step: %d\n", set_data->time_step);
		printf("set_data->frame_count: %d\n", set_data->frame_count);
		printf("set_data->approved: %d\n", set_data->approved);
		dispatcher->set_data = set_data;
	}
	printf("received complete\n");
}
