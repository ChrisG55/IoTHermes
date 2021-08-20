/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "config.h"
#include "io.h"
#include "queue_data.h"
#include "source.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/hermes.h"

#if HAVE_NETDB_H
#include <netdb.h>
#else
#error "system does not support addrinfo structure"
#endif /* HAVE_NETDB_H */

#if HAVE_SCHED_H
#include <sched.h>
#endif /* HAVE_SCHED_H */

static void *message_init(struct source_context *ctx, char *port)
{
	struct client_msg_init *cmi;

	if ((cmi = calloc(1, sizeof(*cmi))) == NULL) {
		ctx->error = errno;
		return NULL;
	}
	cmi->hints.ai_family = AF_UNSPEC;
	cmi->hints.ai_socktype = SOCK_STREAM;
	cmi->hints.ai_flags = 0;
	cmi->hints.ai_protocol = 0;
	cmi->nodename = NULL;
	cmi->port = port;
	cmi->queue = ctx->queue;
	cmi->format = &ctx->format;

	return cmi;
}

static int message_send(struct source_context *ctx,
			enum queue_data type,
			void **rmsg,
			void *msg)
{
	int ret, init;
	struct node *node = NULL;
	struct client_msg_init_response *cmir;
	unsigned char t;

	init = 0;
	if (type == CLIENT_MESSAGE_INIT)
		init = 1;
	if ((ret = queue_enq(ctx->client_queue, msg, type, node)) != 0)
		return ret;
	do {
#if HAVE_SCHED_H
		sched_yield();
#endif /* HAVE_SCHED_H */
		msg = queue_deq(ctx->queue, &t, node);
	} while (msg == NULL);
	if (t != CLIENT_MESSAGE_RESPONSE) {
		ret = 1;
	} else if (init) {
		cmir = msg;
		ret = cmir->return_value;
	} else {
		ret = *(int *)msg;
	}
	if (rmsg != NULL)
		*rmsg = msg;
	gdiot_printf("source send: rv = %d\n", ret);

	return ret;
}

static int source_fini(struct source_context *ctx)
{
	return 0;
}

static int source_init(struct source_context *ctx)
{
	int rc;
	void *msg;
	struct client_msg_init_response *cmir;
	char port[HERMES_SERVER_PORT_LEN];

	switch (ctx->type) {
	case SOURCE_CSV:
	case SOURCE_TSV:
		break;
	default:
		return 1;
	}

	snprintf(port, HERMES_SERVER_PORT_LEN, "%hu", HERMES_SERVER_PORT_MIN);
	if ((msg = message_init(ctx, port)) == NULL)
		return 1;
	rc = message_send(ctx, CLIENT_MESSAGE_INIT, (void **)&cmir, msg);
	if (rc != 0)
		return rc;
	ctx->id = cmir->id;
	ctx->id_size = cmir->size;
	gdiot_printf("source init: id = %lu, size = %zu, rv = %d\n", ctx->id,
		     ctx->id_size, cmir->return_value);
	free(cmir);

	return 0;
}

void *source_main(void *c)
{
	struct source_context *ctx = c;
	size_t bytes_read;

	if ((ctx->ret = source_init(ctx)) != 0)
		return &ctx->ret;

	bytes_read = 0;
	if ((bytes_read == 0) && (ctx->error != 0)) {
		ctx->ret = errno;
		return &ctx->ret;
	}

	ctx->ret = source_fini(ctx);

	return &ctx->ret;
}
