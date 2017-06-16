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
	size_t		end;
	t_sortbod	*sorts;
}				msort_param_t;

int			mphore;
int			mflag;
pthread_t	threadpool[8];

void	mswap(t_sortbod *a, t_sortbod *b)
{
	t_sortbod	t;
	t = *a;
	*a = *b;
	*b = t;
}

void	*qmsort(void *params)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	t_sortbod	pivot;
	size_t		count = ((msort_param_t*)params)->end;
	t_sortbod	*sorts = ((msort_param_t*)params)->sorts;
	pthread_t	tid;
	size_t		end;

	end = count - 1;
	if (count > 2)
	{
		i = 0;
		pi = end / 2;
		pivot = sorts[pi];
		l = end;
		while (i < l)
		{
			if (sbod_comp(&pivot, &sorts[i]) > -1)
				i++;
			else
				mswap(&sorts[i], &sorts[--l]);
		}
		mswap(&sorts[--i], &sorts[0]);
		//thread1
		while (1)
		{
			if (mphore < 8 && mflag == 0)
			{
				mflag = 1;
				tid = threadpool[mphore];
				mphore++;
				msort_param_t	param1;
				param1.end = i;
				param1.sorts = sorts;
				pthread_create(&tid, NULL, qmsort, &param1);
				mflag = 0;
				break ;
			}
		}
		//thread2
		while (1)
		{
			if (mphore < 8 && mflag == 0)
			{
				mflag = 1;
				tid = threadpool[mphore];
				mphore++;
				msort_param_t	param2;
				param2.end = count;
				param2.sorts = &sorts[l];	
				pthread_create(&tid, NULL, qmsort, &param2);
				mflag = 0;
				break ;
			}
		}
	}
	else if (count == 2)
	{
		if (sbod_comp(&sorts[0], &sorts[1]) < 0)
			mswap(&sorts[0], &sorts[1]);
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
	mflag = 0;
	if (count > 2)
	{
		i = 0;
		pi = end / 2;
		pivot = sorts[pi];
		l = end;
		while (i < l)
		{
			if (sbod_comp(&pivot, &sorts[i]) > -1)
				i++;
			else
				mswap(&sorts[i], &sorts[--l]);
		}
		mswap(&sorts[--i], &sorts[0]);
		//thread 1
		msort_param_t	param1;
		param1.end = i;
		param1.sorts = sorts;
		qmsort(&param1);
		//thread2
		msort_param_t	param2;
		param2.end = count;
		param2.sorts = &sorts[l];
		pthread_t	tid2;
		qmsort(&param2);
	}
	else if (count == 2)
	{
		if (sbod_comp(&sorts[0], &sorts[1]) < 0)
			mswap(&sorts[0], &sorts[1]);
	}
	return ;
}
