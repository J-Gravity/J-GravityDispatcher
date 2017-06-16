#include <dispatcher.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <pthread.h>

typedef struct	s_sortbod
{
	t_body bod;
	uint64_t morton;
}               t_sortbod;

// method to seperate bits from a given integer 3 positions apart

uint64_t splitBy3(const unsigned int a)
{
	uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
	x = (x | x << 32) & 0x1f00000000ffff;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
	x = (x | x << 16) & 0x1f0000ff0000ff;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
	x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
	x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
	x = (x | x << 2) & 0x1249249249249249;
	return x;
}

uint64_t mortonEncode_magicbits(const unsigned int x, const unsigned int y, const unsigned int z)
{
	uint64_t answer = 0;
	answer |= splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
	return answer;
}

uint64_t morton64(float x, float y, float z)
{
	x = fmin(fmax(x * 2097152.0f, 0.0f), 2097151.0f);
	y = fmin(fmax(y * 2097152.0f, 0.0f), 2097151.0f);
	z = fmin(fmax(z * 2097152.0f, 0.0f), 2097151.0f);
	return (mortonEncode_magicbits((unsigned int)x, (unsigned int)y, (unsigned int)z));
}

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

void	qmsort(msort_param_t *params)
{
	size_t		pi;
	size_t		i;
	size_t		l;
	t_sortbod	pivot;
	size_t		count = params->end;
	t_sortbod	*sorts = params->sorts;
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
				pthread_create(&tid, NULL, &qmsort, &param1);
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
				pthread_create(&tid, NULL, &qmsort, &param2);
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
	return ;
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
