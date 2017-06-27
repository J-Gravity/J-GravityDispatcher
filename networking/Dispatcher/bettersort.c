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
