#ifndef MSORT_H
# define MSORT_H

# define THREAD_THRESHOLD 10000
# define SORTSWITCH_THRESH 100000
# define MSORT_THREADC 8

#include <stdlib.h>
#include <semaphore.h>

extern sem_t *mphore;
extern int (*g_cmp) (void *, void *);

typedef struct	msort_param_s
{
	size_t		start;
	size_t		end;
	void		*sorts;
	size_t		sortsize;
	int			(*cmp) (const void *, const void *);
}		msort_param_t;

void		*msort(void *sorts, size_t count, size_t datasize, int (*cmp) (void *, void *));
void		*tmsort(void *sorts, size_t count, size_t datasize, int (*cmp) (void *, void *));
void		bsort(msort_param_t params);
void		msort_swap(size_t size, char *a, size_t i, size_t l);
void		*qmsort(void *params);
int			semval(sem_t *sem);

//set number of allowed threads using the MSORT_THREADC macro
# define MSORT_ALLOWEDTHREADS (MSORT_THREADC - 1)

#endif
