/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT evaporator.
 */
#include "args.h"
#include "config.h"

#include <stdlib.h>
#include <time.h>

#include "lib/hermes.h"

static const char *help_line = "GDIoT sink node reference implementation";

int main(int argc, char *argv[])
{
	struct hermes_context *ctx;

	conf.help_line = help_line;

	parse_args(argc, argv);

#if HAVE_UNISTD_H && (defined(_DEFAULT_SOURCE) || defined(_XOPEN_SOURCE))
	srand48(time(NULL));
#else
	srand(time(NULL));
#endif /* HAVE_UNISTD_H && (_DEFAULT_SOURCE || _XOPEN_SOURCE) */

	ctx = hermes_init();

	hermes_connect(ctx);
	hermes_send(ctx, NULL);
	hermes_disconnect(ctx);
	hermes_fini(ctx);

	return EXIT_SUCCESS;
}
