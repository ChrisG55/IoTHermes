/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT evaporator.
 */
#include "args.h"
#include "config.h"
#include "exit.h"
#include "gdiot-evap.h"
#include "source.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/hermes.h"

#if HAVE_NETDB_H
#include <netdb.h>
#else
#error "system does not support addrinfo structure"
#endif /* HAVE_NETDB_H */

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#else
#error "system does not support socket"
#endif /* HAVE_SYS_SOCKET_H */

static const char *help_line = "GDIoT sink node reference implementation";

struct source_context *sctx = NULL;
int nb_sources = 0;

int main(int argc, char *argv[])
{
	int n;
	struct hermes_context *ctx;
	struct addrinfo hints;
	char port[HERMES_SERVER_PORT_LEN];

	conf.help_line = help_line;

	parse_args(argc, argv);

#if HAVE_UNISTD_H && (defined(_DEFAULT_SOURCE) || defined(_XOPEN_SOURCE))
	srand48(time(NULL));
#else
	srand(time(NULL));
#endif /* HAVE_UNISTD_H && (_DEFAULT_SOURCE || _XOPEN_SOURCE) */

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;

	n = snprintf(port, HERMES_SERVER_PORT_LEN, "%hu",
		     HERMES_SERVER_PORT_MIN);
	if (n + 1 != HERMES_SERVER_PORT_LEN) {
		if (n < 0)
			return errno2exit();
		return EXIT_FAILURE;
	}

	port[HERMES_SERVER_PORT_LEN - 1] = '\0';
	ctx = hermes_init(&hints, NULL, port, 0, 0, HERMES_SERVER);
	if (ctx == NULL)
		return errno;
	hermes_connect(ctx);
	hermes_send(ctx, NULL);
	hermes_disconnect(ctx);
	if (hermes_fini(ctx) == -1)
		return errno;

	return EXIT_SUCCESS;
}
