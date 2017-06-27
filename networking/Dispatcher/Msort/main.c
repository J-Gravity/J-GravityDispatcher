#include <stdlib.h>
#include <stdio.h>
#include <msort.h>

typedef struct		s_holder
{
	float		x;
	float		y;
	float		z;
	float		t;
	float		p;
	float		o;
	float		a;
	float		b;
	size_t		k;
}					t_holder;

int		mycmp(const void *a, const void *b)
{
	t_holder *c = (t_holder*)a;
	t_holder *q = (t_holder*)b;

	if (c->k > q->k)
		return (1);
	if (c->k == q->k)
		return (0);
	else
		return (-1);
}

int	ccmp(void *a, void *b)
{
	char *x = (char*)a;
	char *y = (char*)b;

	if (*x > * y)
		return (1);
	if (*x == *y)
		return (0);
	if (*x < *y)
		return (-1);
	return (0);
}

t_holder	newholder(int i)
{
	t_holder	one;

	one.x = i;
	one.y = i;
	one.z = i;
	one.t = i;
	one.p = i;
	one.o = i;
	one.a = i;
	one.b = i;
	one.k = i;

	return (one);
}

int		main(void)
{
	t_sortbod *sorts = malloc(sizeof(t_sortbod) * 1000);

	for (int i = 0; i < 1000; i++)
	{
		sorts[i].morton = rand();
	}
	msort(sorts, 1000);
	for(int i = 0; i < 1000; i++)
	{
		printf("%ld\n", sorts[i].morton);
	}
}
