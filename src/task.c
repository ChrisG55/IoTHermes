#include "config.h"
#include "task.h"

#if HAVE_PTHREADS
#include <errno.h>
#endif /* HAVE_PTHREADS */

#if HAVE_PTHREADS
int task_create(void *(*task_func)(void *ctx),
		task_t *restrict task,
		void *restrict ctx)
{
	return pthread_create(task, NULL, task_func, ctx);
}

int task_free(task_t task)
{
	return 0;
}

int task_join(task_t task, void **ret_ptr)
{
	return pthread_join(task, ret_ptr);
}

int task_multijoin(int n, task_t *restrict tasks, void **ret_ptrs, size_t ret_sz)
{
	int i;
	int rc = 0;

	for (i = 0; i < n; i++) {
		if (!tasks[i])
			return EINVAL;
		if ((rc = pthread_join(tasks[i], ret_ptrs)))
			return rc;
		if ((ret_ptrs != NULL) && (ret_sz > 0))
			ret_ptrs += ret_sz;
	}

	return rc;
}

#else
#error "No task implementation available for this system"
#endif /* HAVE_PTHREADS */
