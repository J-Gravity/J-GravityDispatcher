#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <msort.h>

int g_size;
int (*g_cmp) (void *, void *);

#define I (g_size * i)
#define L (g_size * l)
#define PI (g_size * pi)

sem_t	*mphore;

int		semval(sem_t *sem)
{
	int		v;

	sem_getvalue(sem, &v);
	return (v);
}

void	msort_swap(size_t size, char *a, size_t i, size_t l)
{
	char *t = (char*)malloc(sizeof(char) * size);
	
	memcpy(t, &a[i], size);
	memcpy(&a[i], &a[l], size);
	memcpy(&a[l], t, size);
	free(t);
}

void	*tmsort(void *sort, size_t count, size_t datasize, int (*cmp) (void *, void *))
{
	//timsort
	return (sort);
}

void	*qmsort(void *params)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	size_t		sortsize = ((msort_param_t*)params)->sortsize;
	char		*sorts = (char*)(((msort_param_t*)params)->sorts);
	size_t		end = ((msort_param_t*)params)->end;
	size_t		start = ((msort_param_t*)params)->start;
	int (*cmp) (void *, void *) = g_cmp;

	if (start >= end)
		return (0);
	i = start;
	pi = (start + (end - start) / 2);
	l = end;
	while (i <= l)
	{
		while (cmp(&sorts[I], &sorts[PI]) == -1)
			i++;
		while (cmp(&sorts[L], &sorts[PI]) == 1)
			l--;
		if (i >= l)
			break ;
		if (i == pi)
			pi = l;
		else if (l == pi)
			pi = i;
		msort_swap(sortsize, sorts, I, L);
	}
	
	msort_param_t	param1;
	param1.start = start;
	param1.end = L;
	param1.sorts = sorts;
	param1.sortsize = sortsize;
	
	msort_param_t	param2;
	param2.start = (L + sortsize);
	param2.end = end;
	param2.sorts = sorts;
	param2.sortsize = sortsize;
	
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

void	*msort(void *sort, size_t count, size_t datasize, int (*cmp) (void *, void *))
{
	size_t		pi;
	size_t		i;
	size_t		l;
	size_t		end;
	char		*sorts = (char*)sort;

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
	g_size = datasize;
	g_cmp = cmp;
	i = 0;
	end = count - 1;
	pi = end / 2;
	l = end;
	while (i <= l)
	{
		while (cmp(&sorts[I], &sorts[PI]) == -1)
			i++;
		while (cmp(&sorts[L], &sorts[PI]) == 1)
			l--;
		if (i >= l)
			break ;
		if (i == pi)
			pi = l;
		else if (l == pi)
			pi = i;
		msort_swap(datasize, sorts, I, L);
	}
	
	msort_param_t	param1;
	param1.start = 0;
	param1.end = L;
	param1.sorts = sorts;
	param1.sortsize = datasize;
	
	msort_param_t	param2;
	param2.start = (L + datasize);
	param2.end = end;
	param2.sorts = sorts;
	param2.sortsize = datasize;

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
	return (sort);
}
