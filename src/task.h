#ifndef TASK_H
#define TASK_H

#include "config.h"

#if HAVE_PTHREADS
#include <pthread.h>

typedef pthread_t task_t;
#endif /* HAVE_PTHREADS */

int task_create(void *(*task_func)(void *ctx),
		task_t *restrict task,
		void *restrict ctx);
int task_free(task_t task);
int task_join(task_t task, void **ret_ptr);
int task_multijoin(int n, task_t *restrict tasks, void **ret_ptrs, size_t ret_sz);

#endif /* TASK_H */
