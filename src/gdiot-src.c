/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT source.
 */
#include "args.h"
#include "client.h"
#include "config.h"
#include "exit.h"
#include "io.h"
#include "queue.h"
#include "task.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

static const char *help_line = "GDIoT source node reference implementation";

int main(int argc, char *argv[])
{
	int rc;
	int rv = EXIT_SUCCESS;
	int *crc;
	task_t client;
	struct client_context cctx;
	uint32_t *data;

	conf.help_line = help_line;

	parse_args(argc, argv);

#if HAVE_UNISTD_H && (defined(_DEFAULT_SOURCE) || defined(_XOPEN_SOURCE))
	srand48(time(NULL));
#else
	srand(time(NULL));
#endif /* HAVE_UNISTD_H && (_DEFAULT_SOURCE || _XOPEN_SOURCE) */

	if ((rc = queue_init(&cctx.source_queue)) != 0)
		return errno2exit();

	if ((rc = task_create(client_main, &client, (void *)&cctx)) != 0) {
		errno = rc;
		return errno2exit();
	}

	if ((data = calloc(1, sizeof(*data))) == NULL)
		return errno2exit();
	*data = 0xc0ffee00;
	if (queue_enq(&cctx.source_queue, data, 0) != 0) {
		free(data);
		return errno2exit();
	}

	if ((rc = task_join(client, (void **)&crc)) != 0) {
		errno = rc;
		rv = errno2exit();
	} else if (*crc != 0) {
		rv = EXIT_FAILURE;
	}

	task_free(client);

	return rv;
}
