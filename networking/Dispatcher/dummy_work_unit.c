int		main(void)
{
	int			i;
	t_work_unit	*work_unit;
	t_msg		msg;

	i = -1;
	work_unit = malloc(sizeof(t_work_unit));
	work_unit->compute_class = 15;
	work_unit->cell.body_count = 15;
	work_unit->cell.contained_bodies = (t_body **)malloc(sizeof(t_body *) * work_unit->cell.body_count);
	for (int i = 0; i < work_unit->cell.body_count; i++)
	{
		work_unit->cell.contained_bodies[i] = (t_body *)malloc(sizeof(t_body));
		work_unit->cell.contained_bodies[i]->position.x = 123456789.123;
		work_unit->cell.contained_bodies[i]->position.y = 987654321.321;
		work_unit->cell.contained_bodies[i]->position.z = 123456789.123;
		work_unit->cell.contained_bodies[i]->velocity.x = 111111111.222;
		work_unit->cell.contained_bodies[i]->velocity.y = 222222222.333;
		work_unit->cell.contained_bodies[i]->velocity.z = 333333333.444;
		work_unit->cell.contained_bodies[i]->mass = 333333333.444;
	}
	work_unit->cell.cell_as_body.position.x = 55;
	work_unit->cell.cell_as_body.position.y = 55;
	work_unit->cell.cell_as_body.position.z = 55;
	work_unit->cell.cell_as_body.velocity.x = 22;
	work_unit->cell.cell_as_body.velocity.y = 22;
	work_unit->cell.cell_as_body.velocity.z = 22;
	work_unit->cell.cell_as_body.mass = 100;
	//.........adjoining cells........//
	work_unit->adjoining_cells = malloc(sizeof(t_cell));
	work_unit->adjoining_cells_cnt = 10;
	work_unit->adjoining_cells = (t_cell *)malloc(sizeof(t_cell) * work_unit->adjoining_cells_cnt);
	for (int i = 0; i < work_unit->adjoining_cells_cnt; i++)
	{
		work_unit->adjoining_cells[i].body_count = 10;
		work_unit->adjoining_cells[i].contained_bodies =
			(t_body **)malloc(sizeof(t_body *) * work_unit->adjoining_cells[i].body_count);
		for (int x = 0; x < work_unit->adjoining_cells[i].body_count; x++)
		{
			work_unit->adjoining_cells[i].contained_bodies[x] = (t_body *)malloc(sizeof(t_body));
			work_unit->adjoining_cells[i].contained_bodies[x]->position.x = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->position.y = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->position.z = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->velocity.x = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->velocity.y = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->velocity.z = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->mass = i * x;
		}
		work_unit->adjoining_cells->cell_as_body.position.x = i;
		work_unit->adjoining_cells->cell_as_body.position.y = i;
		work_unit->adjoining_cells->cell_as_body.position.z = i;
		work_unit->adjoining_cells->cell_as_body.velocity.x = i;
		work_unit->adjoining_cells->cell_as_body.velocity.y = i;
		work_unit->adjoining_cells->cell_as_body.velocity.z = i;
		work_unit->adjoining_cells->cell_as_body.mass = i;
	}
	msg = serialize_work_unit(work_unit);
	send_message(msg);
	//serializer_identifier(serial_str);
	return (0);
}