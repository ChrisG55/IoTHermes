/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT source.
 */
#include "args.h"
#include "client.h"
#include "exit.h"
#include "io.h"
#include "task.h"

#include <errno.h>
#include <stdlib.h>

static const char *help_line = "GDIoT source node reference implementation";

int main(int argc, char *argv[])
{
	int rc;
	int rv = EXIT_SUCCESS;
	int *crc;
	task_t client;
	struct client_context cctx;

	conf.help_line = help_line;

	parse_args(argc, argv);

	if ((rc = task_create(client_main, &client, (void *)&cctx)) != 0) {
		errno = rc;
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
