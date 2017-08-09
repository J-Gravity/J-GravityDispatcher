#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "msort.h"
#include "dispatcher.h"

int g_size;
int (*g_cmp) (void *, void *);

#define I (i)
#define L (l)
#define PI (pi)

sem_t	*mphore;

int		semval(sem_t *sem)
{
	int		v;

	sem_getvalue(sem, &v);
	return (v);
}

void	msort_swap(t_sortbod *i, t_sortbod *l)
{
	uint64_t	t;

	t = i->morton;
	i->morton = l->morton;
	l->morton = t;
}

void	*tmsort(void *sort, size_t count, size_t datasize, int (*cmp) (void *, void *))
{
	//timsort
	return (sort);
}

int		mcmp(t_sortbod a, uint64_t b)
{
	if (a.morton < b)
		return (-1);
	if (a.morton >= b)
		return (1);
	dprintf(2, "NAN found, tests done <, >=  : %llu || %llu\n", a.morton, b);
	return (0);
}

void	*qmsort(void *params)
{
	uint64_t	pivot;
	size_t		i;
	size_t		l;
	t_sortbod	*sorts = (t_sortbod*)(((msort_param_t*)params)->sorts);
	size_t		end = ((msort_param_t*)params)->end;
	size_t		start = ((msort_param_t*)params)->start;

	if (start >= end)
		return (0);
	i = start;
	pivot = sorts[(start + (end - start) / 2)].morton;
	l = end;
	while (i <= l)
	{
		//if i < pivot i++;
		while (mcmp(sorts[i], pivot) == -1)
			i++;
		//if l >= pivot l--;
		while (mcmp(sorts[l], pivot) == 1)
			l--;
		if (i >= l)
			break ;
		msort_swap(&sorts[i], &sorts[l]);
	}

	msort_param_t	param1;
	param1.start = start;
	param1.end = L;
	param1.sorts = sorts;
	
	msort_param_t	param2;
	param2.start = (l + 1);
	param2.end = end;
	param2.sorts = sorts;
	
	if (semval(mphore) > 0 && L - start > THREAD_THRESHOLD)
	{
		sem_wait(mphore);
		pthread_t tid1;
		pthread_create(&tid1, NULL, qmsort, &param1);
	}
	else
	{
		bsort(param1);
	}
	if (semval(mphore) > 0 && end - L > THREAD_THRESHOLD)
	{
		sem_wait(mphore);
		pthread_t tid2;
		pthread_create(&tid2, NULL, qmsort, &param2);
	}
	else
	{
		bsort(param2);
	}
	sem_post(mphore);
	return (sorts);
}

void	*msort(t_sortbod *sorts, size_t count)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	size_t		end;
	uint64_t	pivot;

	//do sample of data
	//if looks partially sorted dp timsort
	/*
	if (datasorted)
	{
		tmsort(sort, count, datasize);
	}
	*/
	end = count - 1;
	sem_init(mphore, 0, MSORT_ALLOWEDTHREADS);
	i = 0;
	end = count - 1;
	pivot = sorts[end / 2].morton;
	l = end;
	while (i <= l)
	{
		//if i < pivot i++;
		while (mcmp(sorts[i], pivot) == -1)
			i++;
		//if l >= pivot l--;
		while (mcmp(sorts[l], pivot) == 1)
			l--;
		if (i >= l)
			break ;
		msort_swap(&sorts[i], &sorts[l]);
	}
	msort_param_t	param1;
	param1.start = 0;
	param1.end = l;
	param1.sorts = sorts;
	
	msort_param_t	param2;
	param2.start = (l + 1);
	param2.end = end;
	param2.sorts = sorts;

	if (semval(mphore) > 0 && L > THREAD_THRESHOLD)
	{
		sem_wait(mphore);
		pthread_t tid1;
		pthread_create(&tid1, NULL, qmsort, &param1);
	}
	else
	{
		bsort(param1);
	}
	if (semval(mphore) > 0 && end - L > THREAD_THRESHOLD)
	{
		sem_wait(mphore);
		pthread_t tid2;
		pthread_create(&tid2, NULL, qmsort, &param2);
	}
	else
	{
		bsort(param2);
	}	
	return (sorts);
}
