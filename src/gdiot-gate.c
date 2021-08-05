/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT gateway.
 */
#include "args.h"
#include "config.h"
#include "exit.h"
#include "io.h"
#include "queue.h"
#include "server.h"
#include "task.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#if HAVE_SCHED_H
#include <sched.h>
#endif /* HAVE_SCHED_H */

static const char *help_line = "GDIoT intermediate node reference implementation";

int main(int argc, char *argv[])
{
	int rc;
	int rv = EXIT_SUCCESS;
	int *src;
	task_t server;
	struct server_context sctx;
	struct queue queue;
	uint32_t *data;
	unsigned char type;

	conf.help_line = help_line;

	parse_args(argc, argv);

#if HAVE_UNISTD_H && (defined(_DEFAULT_SOURCE) || defined(_XOPEN_SOURCE))
	srand48(time(NULL));
#else
	srand(time(NULL));
#endif /* HAVE_UNISTD_H && (_DEFAULT_SOURCE || _XOPEN_SOURCE) */

	sctx.queue = &queue;
	if ((rc = queue_init(&queue)) != 0)
		return errno2exit();

	if ((rc = task_create(server_main, &server, (void *)&sctx)) != 0) {
		errno = rc;
	        return errno2exit();
	}

	do {
#if HAVE_SCHED_H
		sched_yield();
#endif /* HAVE_SCHED_H */
		data = queue_deq(&queue, &type);
	} while (data == NULL);
	gdiot_printf("server: data = 0x%"PRIx32"\n", *data);
	free(data);

	if ((rc = task_join(server, (void **)&src)) != 0) {
		errno = rc;
		rv = errno2exit();
	} else if (*src != 0) {
		rv = EXIT_FAILURE;
	}

	task_free(server);

	return rv;
}
