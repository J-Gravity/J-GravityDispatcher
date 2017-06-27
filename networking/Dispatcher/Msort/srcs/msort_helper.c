#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <msort.h>


void	bsort(msort_param_t params)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	char		*sorts = (char*)(params.sorts);
	size_t		sortsize = params.sortsize;
	size_t		end = params.end;
	size_t		start = params.start;
	int (*cmp) (void *, void *) = g_cmp;
	
	if (start >= end)
		return ;
	i = start;
	pi = (start + (end - start) / 2);
	l = end;
	while (i <= l)
	{
		while (cmp(&sorts[i * sortsize], &sorts[pi * sortsize]) == -1)
			i++;
		while (cmp(&sorts[l * sortsize], &sorts[pi * sortsize]) == 1)
			l--;
		if (i >= l)
			break ;
		if (i == pi)
			pi = l;
		else if (l == pi)
			pi = i;
		msort_swap(sortsize, sorts, i * sortsize, l * sortsize);
	}
	
	msort_param_t	param1;
	param1.start = start;
	param1.end = l * sortsize;
	param1.sorts = sorts;
	param1.sortsize = sortsize;
	
	msort_param_t	param2;
	param2.start = ((l * sortsize) + sortsize);
	param2.end = end;
	param2.sorts = sorts;
	param2.sortsize = sortsize;
	
	if (semval(mphore) > 0 && (l * sortsize) - start > THREAD_THRESHOLD)
	{
		sem_wait(mphore);
		pthread_t tid1;;
		pthread_create(&tid1, NULL, qmsort, &param1);
	}
	else
	{
		bsort(param1);
	}
	if (semval(mphore) > 0 && end - (l * sortsize) > THREAD_THRESHOLD)
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
