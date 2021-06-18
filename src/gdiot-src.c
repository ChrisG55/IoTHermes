/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT source.
 */
#include "args.h"

#include <stdlib.h>

#include "lib/hermes.h"

static const char *help_line = "GDIoT source node reference implementation";

int main(int argc, char *argv[])
{
	struct hermes_context *ctx;

	conf.help_line = help_line;

	parse_args(argc, argv);

	ctx = hermes_init();
	hermes_connect(ctx);
	hermes_send(ctx, NULL);
	hermes_disconnect(ctx);
	hermes_fini(ctx);

	return EXIT_SUCCESS;
}
