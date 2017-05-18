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
		workunit->local_bodies[i].position.x = 123456789.123;
		workunit->local_bodies[i].position.y = 987654321.321;
		workunit->local_bodies[i].position.z = 123456789.123;
		workunit->local_bodies[i].velocity.x = 111111111.222;
		workunit->local_bodies[i].velocity.y = 222222222.333;
		workunit->local_bodies[i].velocity.z = 333333333.444;
	}
	workunit->neighborcount = 10;
	workunit->neighborhood = (t_body *)malloc(sizeof(t_body) * workunit->neighborcount);
	for (int i = 0; i < workunit->neighborcount; i++)
	{
		workunit->neighborhood[i].position.x = i * 5;
		workunit->neighborhood[i].position.y = i * 5;
		workunit->neighborhood[i].position.z = i * 5;
		workunit->neighborhood[i].velocity.x = i * 5;
		workunit->neighborhood[i].velocity.y = i * 5;
		workunit->neighborhood[i].velocity.z = i * 5;
	}
	workunit->force_bias = (cl_float4){{1.0, 5.0, 6.0, 9.0}};
	msg = serialize_workunit(workunit);
	deserialize_workunit(msg);
	return (0);
}