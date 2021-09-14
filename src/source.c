/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "config.h"
#include "csv.h"
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

static void *message_fini(struct source_context *ctx)
{
	struct client_msg_fini *cmf;

	if ((cmf = calloc(1, sizeof(*cmf))) == NULL) {
		ctx->error = errno;
		return cmf;
	}
	cmf->id = ctx->id;
	cmf->size = ctx->id_size;

	return cmf;
}

static void *message_data(struct source_context *ctx, void *data)
{
	struct client_msg_data *cmd;

	if ((cmd = calloc(1, sizeof(*cmd))) == NULL) {
		ctx->error = errno;
		return NULL;
	}
	cmd->id = ctx->id;
	cmd->size = ctx->id_size;
	cmd->data = data;

	return cmd;
}

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
	int rc;
	void *msg;

	switch (ctx->type) {
	case SOURCE_CSV:
	case SOURCE_TSV:
		csv_finish(ctx);
		break;
	default:
		return 1;
	}

	if ((msg = message_fini(ctx)) == NULL)
		return 1;
	if ((rc = message_send(ctx, CLIENT_MESSAGE_FINI, NULL, msg)) != 0)
		return 1;
	gdiot_printf("source fini: rv = %d\n", rc);

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
		if (csv_initialize(ctx, BUFSIZ) != 0)
			return 1;

		ctx->read = csv_data;
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
	void *msg;
	void *data;
	unsigned char type;
	struct node *node = NULL;

	if ((ctx->ret = source_init(ctx)) != 0)
		return &ctx->ret;

	data = ctx->read(ctx);
	if ((data == NULL) && (ctx->error != 0)) {
		ctx->ret = errno;
		return &ctx->ret;
	}

	/* free(data); */
	/* if ((data = calloc(1, sizeof(int))) == NULL) { */
	/* 	ctx->error = errno; */
	/* 	ctx->ret = 1; */
	/* 	return &ctx->ret; */
	/* } */
	/* *(int *)data = 0xc0ffee00; */
	if ((msg = message_data(ctx, data)) == NULL) {
		ctx->ret = 1;
		return &ctx->ret;
	}
	if ((ctx->ret = message_send(ctx, CLIENT_MESSAGE_DATA, NULL, msg)) != 0)
		return &ctx->ret;

	ctx->ret = source_fini(ctx);

	return &ctx->ret;
}
