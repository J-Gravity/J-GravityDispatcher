#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <pthread.h>
#include "dispatcher.h"
#include <math.h>
#include <limits.h>
#include "lz4.h"
#include "transpose.h"

#define THREAD_THRESHOLD 10000

int sbod_comp(const void *a, const void *b)
{
	uint64_t ma = ((t_sortbod *)a)->morton;
	uint64_t mb = ((t_sortbod *)b)->morton;
	if (ma > mb)
		return 1;
	if (ma == mb)
		return 0;
	else
		return -1;
}

typedef struct	msort_param_s
{
	size_t		start;
	size_t		end;
	t_sortbod	*sorts;
}				msort_param_t;

volatile int	mphore;

void	mswap(t_sortbod *a, t_sortbod *b)
{
	t_sortbod	t;
	t = *a;
	*a = *b;
	*b = t;
}

void	bsort(void *params)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	t_sortbod	pivot;
	t_sortbod	*sorts = ((msort_param_t*)params)->sorts;
	size_t		end = ((msort_param_t*)params)->end;
	size_t		start = ((msort_param_t*)params)->start;
	
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

void	*qmsort(void *params)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	t_sortbod	pivot;
	t_sortbod	*sorts = ((msort_param_t*)params)->sorts;
	pthread_t	tid;
	size_t		end = ((msort_param_t*)params)->end;
	size_t		start = ((msort_param_t*)params)->start;

	if (start >-= end)
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
		pthread_t tid1;
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
	mphore--;
	return (sorts);
}

void	msort(t_sortbod *sorts, size_t count)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	t_sortbod	pivot;
	size_t		end;

	end = count - 1;
	mphore = 0;
	i = 0;
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
		pthread_t tid1;
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
