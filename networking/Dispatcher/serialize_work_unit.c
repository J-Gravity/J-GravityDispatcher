/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dispatcher.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssmith <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/14 20:43:09 by ssmith            #+#    #+#             */
/*   Updated: 2017/05/14 20:43:11 by ssmith           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"

char	*itob(int value)
{
	unsigned int	i;
	char			*string;

	i = 0;
	string = calloc(1, sizeof(int));
	while (i < sizeof(int))
	{
		string[i] = ((char *)(&value))[i];
		i++;
	}
	return string;	
}

char	*ftob(float value)
{
	char *string;

	string = calloc(1, sizeof(float));
	for (unsigned int i = 0; i < sizeof(float); i++)
		string[i] = ((char *)(&value))[i];
	return string;	
}

void	strbjoin(t_msg *msg, char const *s2, size_t size)
{
	char	*copy;
	int		i;

	i = 0;
	copy = (char *)malloc(msg->size);
	for (int x = 0; x < msg->size; x++)
		copy[x] = msg->data[x];
	msg->data = (char *)calloc(1, msg->size + size);
	while (i < msg->size)
	{
		msg->data[i] = copy[i];
		i++;
	}
	for (unsigned int x = 0; x < size; x++)
	{
		msg->data[i] = s2[x];
		i++;
	}
	msg->size += size;
}

t_msg	serialize_work_unit(t_work_unit *work_unit)
{
	t_msg	*msg;

	msg = malloc(sizeof(msg));
	msg->data = itob(work_unit->compute_class);
	msg->size = 4;
	strbjoin(msg, itob(work_unit->cell.body_count), sizeof(int));
	for (int i = 0; i< work_unit->cell.body_count; i++)
	{
		strbjoin(msg, ftob(work_unit->cell.contained_bodies[i]->position.x), sizeof(float));
		strbjoin(msg, ftob(work_unit->cell.contained_bodies[i]->position.y), sizeof(float));
		strbjoin(msg, ftob(work_unit->cell.contained_bodies[i]->position.z), sizeof(float));
		strbjoin(msg, ftob(work_unit->cell.contained_bodies[i]->velocity.x), sizeof(float));
		strbjoin(msg, ftob(work_unit->cell.contained_bodies[i]->velocity.y), sizeof(float));
		strbjoin(msg, ftob(work_unit->cell.contained_bodies[i]->velocity.z), sizeof(float));
		strbjoin(msg, ftob(work_unit->cell.contained_bodies[i]->mass), sizeof(float));
	}
	strbjoin(msg, ftob(work_unit->cell.cell_as_body.position.x), sizeof(float));
	strbjoin(msg, ftob(work_unit->cell.cell_as_body.position.y), sizeof(float));
	strbjoin(msg, ftob(work_unit->cell.cell_as_body.position.z), sizeof(float));
	strbjoin(msg, ftob(work_unit->cell.cell_as_body.velocity.x), sizeof(float));
	strbjoin(msg, ftob(work_unit->cell.cell_as_body.velocity.y), sizeof(float));
	strbjoin(msg, ftob(work_unit->cell.cell_as_body.velocity.z), sizeof(float));
	strbjoin(msg, ftob(work_unit->cell.cell_as_body.mass), sizeof(float));
	strbjoin(msg, itob(work_unit->adjoining_cells_cnt), sizeof(int));
	for (int i = 0; i < work_unit->adjoining_cells_cnt; i++)
	{
		strbjoin(msg, itob(work_unit->adjoining_cells[i].body_count), sizeof(int));
		for (int x = 0; x < work_unit->adjoining_cells[i].body_count; x++)
		{
			strbjoin(msg, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->position.x), sizeof(float));
			strbjoin(msg, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->position.y), sizeof(float));
			strbjoin(msg, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->position.z), sizeof(float));
			strbjoin(msg, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->velocity.x), sizeof(float));
			strbjoin(msg, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->velocity.y), sizeof(float));
			strbjoin(msg, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->velocity.z), sizeof(float));
			strbjoin(msg, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->mass), sizeof(float));
		}
		strbjoin(msg, ftob(work_unit->adjoining_cells[i].cell_as_body.position.x), sizeof(float));
		strbjoin(msg, ftob(work_unit->adjoining_cells[i].cell_as_body.position.y), sizeof(float));
		strbjoin(msg, ftob(work_unit->adjoining_cells[i].cell_as_body.position.z), sizeof(float));
		strbjoin(msg, ftob(work_unit->adjoining_cells[i].cell_as_body.velocity.x), sizeof(float));
		strbjoin(msg, ftob(work_unit->adjoining_cells[i].cell_as_body.velocity.y), sizeof(float));
		strbjoin(msg, ftob(work_unit->adjoining_cells[i].cell_as_body.velocity.z), sizeof(float));
		strbjoin(msg, ftob(work_unit->adjoining_cells[i].cell_as_body.mass), sizeof(float));
	}
	return (*msg);
}
