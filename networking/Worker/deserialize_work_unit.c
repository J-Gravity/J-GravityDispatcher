/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   worker.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/08 16:08:11 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/14 20:43:43 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

int		btoi(char *str)
{
	int				ret;

	for (unsigned int i = 0; i < sizeof(int); i++)
		((char *)(&ret))[i] = str[i];
	return (ret);
}

float	btof(char *str)
{
	float	ret;

	for (unsigned int i = 0; i < sizeof(float); i++)
		((char *)(&ret))[i] = str[i];
	return (ret);
}

t_work_unit	*deserialize_work_unit(t_msg msg)
{
	int			i;
	t_work_unit	*work_unit;

	i = 0;
	work_unit = malloc(sizeof(t_work_unit));
	work_unit->compute_class = btoi(msg.data += 4);
	work_unit->cell.body_count = btoi(msg.data += 4);
	work_unit->cell.contained_bodies = (t_body **)malloc(sizeof(t_body *) * work_unit->cell.body_count);
	for (int i = 0; i < work_unit->cell.body_count; i++)
	{
		work_unit->cell.contained_bodies[i] = (t_body *)malloc(sizeof(t_body));
		work_unit->cell.contained_bodies[i]->position.x = btof(msg.data += 4);
		work_unit->cell.contained_bodies[i]->position.y = btof(msg.data += 4);
		work_unit->cell.contained_bodies[i]->position.z = btof(msg.data += 4);
		work_unit->cell.contained_bodies[i]->velocity.x = btof(msg.data += 4);
		work_unit->cell.contained_bodies[i]->velocity.y = btof(msg.data += 4);
		work_unit->cell.contained_bodies[i]->velocity.z = btof(msg.data += 4);
		work_unit->cell.contained_bodies[i]->mass = btof(msg.data += 4);
	}
	work_unit->cell.cell_as_body.position.x = btof(msg.data += 4);
	work_unit->cell.cell_as_body.position.y = btof(msg.data += 4);
	work_unit->cell.cell_as_body.position.z = btof(msg.data += 4);
	work_unit->cell.cell_as_body.velocity.x = btof(msg.data += 4);
	work_unit->cell.cell_as_body.velocity.y = btof(msg.data += 4);
	work_unit->cell.cell_as_body.velocity.z = btof(msg.data += 4);
	work_unit->cell.cell_as_body.mass = btof(msg.data += 4);
	
	work_unit->adjoining_cells_cnt = btoi(msg.data += 4);
	work_unit->adjoining_cells = (t_cell *)malloc(sizeof(t_cell) * work_unit->adjoining_cells_cnt);
	for (int i = 0; i < work_unit->adjoining_cells_cnt; i++)
	{
		work_unit->adjoining_cells[i].body_count = btoi(msg.data += 4);
		work_unit->adjoining_cells[i].contained_bodies =
			(t_body **)malloc(sizeof(t_body *) * work_unit->adjoining_cells[i].body_count);
		for (int x = 0; x < work_unit->adjoining_cells[i].body_count; x++)
		{
			work_unit->adjoining_cells[i].contained_bodies[x] = (t_body *)malloc(sizeof(t_body));
			work_unit->adjoining_cells[i].contained_bodies[x]->position.x = btof(msg.data += 4);
			work_unit->adjoining_cells[i].contained_bodies[x]->position.y = btof(msg.data += 4);
			work_unit->adjoining_cells[i].contained_bodies[x]->position.z = btof(msg.data += 4);
			work_unit->adjoining_cells[i].contained_bodies[x]->velocity.x = btof(msg.data += 4);
			work_unit->adjoining_cells[i].contained_bodies[x]->velocity.y = btof(msg.data += 4);
			work_unit->adjoining_cells[i].contained_bodies[x]->velocity.z = btof(msg.data += 4);
			work_unit->adjoining_cells[i].contained_bodies[x]->mass = btof(msg.data += 4);
		}
		work_unit->adjoining_cells[i].cell_as_body.position.x = btof(msg.data += 4);
		work_unit->adjoining_cells[i].cell_as_body.position.y = btof(msg.data += 4);
		work_unit->adjoining_cells[i].cell_as_body.position.z = btof(msg.data += 4);
		work_unit->adjoining_cells[i].cell_as_body.velocity.x = btof(msg.data += 4);
		work_unit->adjoining_cells[i].cell_as_body.velocity.y = btof(msg.data += 4);
		work_unit->adjoining_cells[i].cell_as_body.velocity.z = btof(msg.data += 4);
		work_unit->adjoining_cells[i].cell_as_body.mass = btof(msg.data += 4);
	}
	return (work_unit);
}
