/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT gateway.
 */
#include "args.h"
#include "exit.h"
#include "server.h"
#include "task.h"

#include <errno.h>
#include <stdlib.h>

static const char *help_line = "GDIoT intermediate node reference implementation";

int main(int argc, char *argv[])
{
	int rc;
	int rv = EXIT_SUCCESS;
	int *src;
	task_t server;
	struct server_context sctx;

	conf.help_line = help_line;

	parse_args(argc, argv);

	if ((rc = task_create(server_main, &server, (void *)&sctx)) != 0) {
		errno = rc;
	        return errno2exit();
	}

	if ((rc = task_join(server, (void **)&src)) != 0) {
		errno = rc;
		rv = errno2exit();
	} else if (*src != 0) {
		rv = EXIT_FAILURE;
	}

	task_free(server);

	return rv;
}
