/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT source.
 */
#include "args.h"
#include "client.h"
#include "config.h"
#include "exit.h"
#include "gdiot-src.h"
#include "io.h"
#include "queue.h"
#include "queue_data.h"
#include "source.h"
#include "task.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "lib/hermes.h"

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#if HAVE_NETDB_H
#include <netdb.h>
#else
#error "system does not support addrinfo structure"
#endif /* HAVE_NETDB_H */

#if HAVE_SCHED_H
#include <sched.h>
#endif /* HAVE_SCHED_H */

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#else
#error "system does not support socket"
#endif /* HAVE_SYS_SOCKET_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

static const char *help_line = "GDIoT source node reference implementation";

struct source_context *sctx = NULL;
int nb_sources = 0;

int main(int argc, char *argv[])
{
	int rc;
	int rv = EXIT_SUCCESS;
	int *crc;
	task_t client, source;
	struct client_context cctx;
	struct queue queue;
	int i;

	conf.help_line = help_line;

	parse_args(argc, argv);

#if HAVE_UNISTD_H && (defined(_DEFAULT_SOURCE) || defined(_XOPEN_SOURCE))
	srand48(time(NULL));
#else
	srand(time(NULL));
#endif /* HAVE_UNISTD_H && (_DEFAULT_SOURCE || _XOPEN_SOURCE) */

	if (queue_init(&queue) != 0)
		return errno2exit();

	if ((rc = queue_init(&cctx.source_queue)) != 0)
		return errno2exit();

	cctx.hints.ai_family = AF_UNSPEC;
	cctx.hints.ai_socktype = SOCK_STREAM;
	cctx.hints.ai_flags = 0;
	cctx.hints.ai_protocol = 0;
	cctx.nodename = NULL;

	if ((rc = task_create(client_main, &client, (void *)&cctx)) != 0) {
		errno = rc;
		return errno2exit();
	}

	if (sctx == NULL) {
		errno = EINVAL;
		return errno2exit();
	}
	for (i = 0; i < nb_sources; i++) {
		sctx[i].queue = &queue;
		sctx[i].client_queue = &cctx.source_queue;
	}

	if ((rc = task_create(source_main, &source, (void *)&sctx[0])) != 0) {
		errno = rc;
		return errno2exit();
	}

	if ((rc = task_join(source, (void **)&crc)) != 0) {
		errno = rc;
		rv = errno2exit();
	} else if (*crc != 0) {
		rv = EXIT_FAILURE;
	}

	if ((rc = task_join(client, (void **)&crc)) != 0) {
		errno = rc;
		rv = errno2exit();
	} else if (*crc != 0) {
		rv = EXIT_FAILURE;
	}

	task_free(client);
	task_free(source);

	return rv;
}
