#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <msort.h>

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
