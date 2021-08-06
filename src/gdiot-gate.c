/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT gateway.
 */
#include "args.h"
#include "config.h"
#include "exit.h"
#include "gdiot-gate.h"
#include "io.h"
#include "queue.h"
#include "server.h"
#include "source.h"
#include "task.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

static unsigned int port_next = HERMES_SERVER_PORT_MIN;
static const char *help_line = "GDIoT intermediate node reference implementation";

struct source_context *sctx = NULL;
int nb_sources = 0;

/*
 * Return a string representation of the next port number.
 * Upon successful completion returns a pointer to a string representing the
 * next port number, otherwise, a NULL pointer is returned and errno is set to
 * indicate the error.
 */
static char *get_next_port(void)
{
	char *buf;

	if ((buf = calloc(HERMES_SERVER_PORT_LEN, sizeof(*buf))) == NULL)
		return buf;

	snprintf(buf, HERMES_SERVER_PORT_LEN, "%hu", port_next++);
	if (port_next == 0)
		port_next = HERMES_SERVER_PORT_MIN;

	return buf;
}

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

	if ((sctx.port = get_next_port()) == NULL)
		return errno2exit();

	sctx.queue = &queue;
	if ((rc = queue_init(&queue)) != 0)
		return errno2exit();

	sctx.hints.ai_family = AF_UNSPEC;
	sctx.hints.ai_socktype = SOCK_STREAM;
	sctx.hints.ai_flags = AI_PASSIVE;
	sctx.hints.ai_protocol = 0;
	sctx.nodename = NULL;

	if ((rc = task_create(server_main, &server, (void *)&sctx)) != 0) {
		errno = rc;
	        return errno2exit();
	}

	do {
#if HAVE_SCHED_H
		sched_yield();
#endif /* HAVE_SCHED_H */
		data = queue_deq(&queue, &type, NULL);
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
