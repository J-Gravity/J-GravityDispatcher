#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "msort.h"


void	bsort(msort_param_t params)
{
	size_t		pivot;
	size_t		i;
	size_t		l;
	t_sortbod	*sorts = (t_sortbod*)(params.sorts);
	size_t		end = params.end;
	size_t		start = params.start;
	
	if (start >= end)
		return ;
	i = start;
	pivot = sorts[(start + (end - start) / 2)].morton;
	l = end;
	while (i <= l)
	{
		//if i < pivot i++;
		while (cmp(sorts[i], pivot) == -1)
			i++;
		//if l >= pivot l--;
		while (cmp(sorts[l], pivot) == 1)
			l--;
		if (i >= l)
			break ;
		msort_swap(datasize, sorts, i, l);
	}
	
	msort_param_t	param1;
	param1.start = start;
	param1.end = l;
	param1.sorts = sorts;
	
	msort_param_t	param2;
	param2.start = (l + 1);
	param2.end = end;
	param2.sorts = sorts;
	
	if (semval(mphore) > 0 && (l - start) > THREAD_THRESHOLD)
	{
		sem_wait(mphore);
		pthread_t tid1;;
		pthread_create(&tid1, NULL, qmsort, &param1);
	}
	else
	{
		bsort(param1);
	}
	if (semval(mphore) > 0 && (end - l) > THREAD_THRESHOLD)
	{
		sem_wait(mphore);
		pthread_t tid2;
		pthread_create(&tid2, NULL, qmsort, &param2);
	}
	else
	{
		bsort(param2);
	}
	return ;
}
