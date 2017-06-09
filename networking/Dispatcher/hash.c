/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   jaccard.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elee <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/25 16:49:38 by elee              #+#    #+#             */
/*   Updated: 2017/05/26 16:02:04 by elee             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "dispatcher.h"
#include <stdint.h>

void	free_dict(t_dict *dict)
{
	unsigned int	i;
	t_pair			*head;
	t_pair			*tmp;
	t_pair			*sub;

	if (dict)
	{
		i = 0;
		while (i < dict->size)
		{
			head = dict->table[i];
			while (head != NULL)
			{
				sub = head->subkeys;
				while (sub)
				{
					tmp = sub;
					sub = sub->next_key;
					free(tmp);
				}
				tmp = head;
				head = head->next_key;
				free(tmp);
			}
			i++;
		}
		free(dict->table);
		free(dict);
	}
}

unsigned int	hash(t_dict *dict, size_t val)
{
	val = (val ^ (val >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
	val = (val ^ (val >> 27)) * UINT64_C(0x94d049bb133111eb);
	val = val ^ (val >> 31);
	return (val % dict->size);
}

t_dict	*create_dict(unsigned int size)
{
	t_dict	*dict;
	size_t	i;

	if (!(dict = (t_dict*)malloc(sizeof(t_dict))))
		return (NULL);
	if (!(dict->table = (t_pair**)malloc(sizeof(t_pair*) * size)))
	{
		free(dict);
		return (NULL);
	}
	i = 0;
	while (i < size)
		dict->table[i++] = NULL;
	dict->size = size;
	return (dict);
}

t_pair	*create_pair(size_t key)
{
	t_pair	*new;

	if (!(new = (t_pair*)malloc(sizeof(t_pair))))
		return (NULL);
	new->key = key;
	new->next_key = NULL;
	new->subkeys = NULL;
	return (new);
}

void	dict_insert(t_dict *dict, t_tree *cell, size_t subkey)
{
	size_t			key;
	unsigned int	hash_val;
	t_pair			*new;
	t_pair			*curr;
	t_pair			*last;

	if (cell->count == 0)
	{
		// printf("cell was empty\n");
		return ;
	}
	key = (size_t)(cell);
	hash_val = hash(dict, key);
	// printf("hash val was %u\n", hash_val);
	curr = dict->table[hash_val];
	last = curr;
	//run through keys until matching or null
	while (curr && key != curr->key)
	{
		//printf("moving forward\n");
		last = curr;
		curr = curr->next_key;
	}
	if (!last) //there was nothing at this hash (last never changed from curr and curr was null at start)
	{
		//printf("empty entry\n");
		//setup new first key & subkey
		dict->table[hash_val] = create_pair(key);
		dict->table[hash_val]->subkeys = create_pair(subkey);
	}
	else if (!curr) //there was an entry at this hash but none for this actual cell. ordinary collision.
	{
		//printf("regular collision\n");
		//add key with subkey at end of keylist
		last->next_key = create_pair(key);
		last->next_key->subkeys = create_pair(subkey);
	}
	else //we've accounted for this cell already. good collision.
	{
		//printf("overlap\n");
		//add a subkey (there guaranteed is one, just push to top of list)
		last = curr->subkeys;
		curr->subkeys = create_pair(subkey);
		curr->subkeys->next_key = last;
	}
}

int *make_subkey_arr(t_pair *subkey, int *len)
{
	int count = 0;
	t_pair *next = subkey;
	while (next)
	{
		next = next->next_key;
		count++;
	}
	int *arr = calloc(count, sizeof(int));
	count = 0;
	next = subkey;
	while (next)
	{
		arr[count] = next->key;
		next = next->next_key;
		count++;
	}
	*len = count;
	// printf("count %d\n",count);
	// for (int i = 0; i < count; i++)
	// 	printf("%d ", arr[i]);
	// printf("\n");
	return (arr);
}

t_bundle *bundle_dict(t_dict *dict, t_pair *ids)
{
	//simplest first: turn ids linked list into an int array
	int len = 0;
	t_pair *next = ids;
	while (next)
	{
		next = next->next_key;
		len++;
	}
	int *keys = calloc(len, sizeof(int));
	int keycount = len;
	next = ids;
	while (next)
	{
		t_pair *temp;
		len--;
		keys[len] = next->key;
		temp = next;
		next = next->next_key;
		free(temp);
	}
	//count cells in union so we can malloc
	int count = 0;
	t_pair *curr;
	for (int i = 0; i < dict->size; i++)
	{
		curr = dict->table[i];
		while (curr)
		{
			count++;
			curr = curr->next_key;
		}
	}
	//sweep again and populate the array of cells and array of subkey arrays
	t_tree **cells = calloc(count + 1, sizeof(t_tree *));
	int **matches = calloc(count + 1, sizeof(int *));
	int *matches_counts = calloc(count, sizeof(int));
	int j = 0;
	for (int i = 0; i < dict->size; i++)
	{
		curr = dict->table[i];
		while (curr)
		{
			cells[j] = (t_tree *)curr->key;
			matches[j] = make_subkey_arr(curr->subkeys, &(matches_counts[j]));
			curr = curr->next_key;
			j++;
		}
	}

	//bundle that dict
	t_bundle *b = calloc(1, sizeof(t_bundle));
	b->cells = cells;
	b->keys = keys;
	b->matches = matches;
	b->keycount = keycount;
	b->cellcount = count;
	b->matches_counts = matches_counts;
	free_dict(dict);
	return (b);
}

int		dict_search(t_dict *dict, t_tree *cell, size_t subkey)
{
	size_t			key;
	unsigned int	hash_val;
	t_pair			*pair;
	t_pair 			*sk;

	key = (size_t)cell;
	hash_val = hash(dict, key);
	pair = dict->table[hash_val];
	while (pair != NULL && key != pair->key)
		pair = pair->next_key;
	if (!pair)
		return 0;
	sk = pair->subkeys; //known not null
	while (sk != NULL && sk->key != subkey)
		sk = sk->next_key;
	if (!sk)
		return 0;
	return 1;
}