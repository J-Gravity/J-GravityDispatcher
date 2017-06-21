#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <pthread.h>
#include "dispatcher.h"

typedef struct	msort_param_s
{
	size_t		start;
	size_t		end;
	t_sortbod	*sorts;
}				msort_param_t;


#define THREAD_THRESHOLD 10000

void	bsort(void *params)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	t_sortbod	pivot;
	t_sortbod	*sorts = ((msort_param_t*)params)->sorts;
	size_t		end = ((msort_param_t*)params)->end;
	size_t		start = ((msort_param_t*)params)->start;

	if (start >= end)
		return (0);
	i = start;
	pi = (start + (end - start) / 2);
	pivot = sorts[pi];
	l = end;
	while (i <= l)
	{
		while (sbod_comp(&pivot, &sorts[i]) == 1)
			i++;
		while (sbod_comp(&pivot, &sorts[l]) == -1)
			l--;
		if (i >= l)
			break ;
		mswap(&sorts[i], &sorts[l]);
	}
	
	msort_param_t	param1;
	param1.start = start;
	param1.end = l;
	param1.sorts = sorts;
	
	msort_param_t	param2;
	param2.start = (l + 1);
	param2.end = end;
	param2.sorts = sorts;
	
	if (mphore < 8 && l - start > THREAD_THRESHOLD)
	{
		mphore++;
		pthread_t tid1;;
		pthread_create(&tid1, NULL, qmsort, &param1);
	}
	else
	{
		bsort(&param1);
	}
	if (mphore < 8 && end - l > THREAD_THRESHOLD)
	{
		mphore++;
		pthread_t tid2;
		pthread_create(&tid2, NULL, qmsort, &param2);
	}
	else
	{
		bsort(&param2);
	}
	return ;
}