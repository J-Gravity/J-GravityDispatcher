
#include "dispatcher.h"

int		main(void)
{
	int			i;
	t_workunit	*workunit;
	t_msg		msg;

	i = -1;
	workunit = malloc(sizeof(t_workunit));
	workunit->id = 15;
	workunit->localcount = 15;
	workunit->local_bodies = (t_body *)malloc(sizeof(t_body) * workunit->localcount);
	for (int i = 0; i < workunit->localcount; i++)
	{
		workunit->local_bodies[i].position = (cl_float4){{1.0, 5.0, 6.0, 9.0}};
		workunit->local_bodies[i].velocity = (cl_float4){{1.0, 5.0, 6.0, 9.0}};
	}
	workunit->neighborcount = 10;
	workunit->neighborhood = (t_body *)malloc(sizeof(t_body) * workunit->neighborcount);
	for (int i = 0; i < workunit->neighborcount; i++)
	{
		workunit->neighborhood[i].position = (cl_float4){{1.0, 5.0, 6.0, 9.0}};
		workunit->neighborhood[i].velocity = (cl_float4){{1.0, 5.0, 6.0, 9.0}};
	}
	workunit->force_bias = (cl_float4){{1.0, 5.0, 6.0, 9.0}};
	msg = serialize_workunit(workunit);
	deserialize_workunit(msg);
	return (0);
}
