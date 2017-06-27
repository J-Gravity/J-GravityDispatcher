#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "msort.h"
#include <stdio.h>

void	bsort(msort_param_t params)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	t_sortbod	*sorts = (char*)(params.sorts);
	size_t		end = params.end;
	size_t		start = params.start;
	t_sortbod	temp;
	
	if (start >= end)
		return ;
	i = start;
	pi = (start + (end - start) / 2);
	l = end;
	while (i <= l)
	{
		while (sbod_comp(&sorts[i], &sorts[pi]) == -1)
			i++;
		while (sbod_comp(&sorts[l], &sorts[pi]) == 1)
			l--;
		if (i >= l)
			break ;
		if (i == pi)
			pi = l;
		else if (l == pi)
			pi = i;
		temp = sorts[i];
		sorts[i] = sorts[l];
		sorts[l] = temp;
	}
	msort_param_t	param1;
	param1.start = start;
	param1.end = l;
	param1.sorts = sorts;
	
	msort_param_t	param2;
	param2.start = l + 1;
	param2.end = end;
	param2.sorts = sorts;
	
	if (semval(mphore) > 0 && l - start > THREAD_THRESHOLD)
	{
		sem_wait(mphore);
		pthread_t tid1;;
		pthread_create(&tid1, NULL, qmsort, &param1);
	}
	else
	{
		bsort(param1);
	}
	if (semval(mphore) > 0 && end - l > THREAD_THRESHOLD)
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
