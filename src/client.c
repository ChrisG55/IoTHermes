#include "client.h"
#include "config.h"
#include "hash.h"
#include "queue.h"
#include "queue_data.h"
#include "streamid.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if HAVE_SCHED_H
#include <sched.h>
#endif /* HAVE_SCHED_H */

#include "lib/hermes.h"

struct source_context {
	struct hermes_context *hctx;
	struct queue *queue;
};

static int client_init(struct client_context *ctx)
{
	ctx->registry = hash_create(UINT_MAX, streamid_compare, NULL);
	if (ctx->registry == NULL)
		return 1;

	return 0;
}

void *client_main(void *c)
{
	struct client_context *ctx = c;
	uint32_t *data;
	unsigned char type;
	int done = 0;
	struct hermes_context *hctx;
	hnode_t *node;
	struct source_context *sctx;

	if ((ctx->ret = client_init(ctx)) != 0)
		return &ctx->ret;

	while (!done) {
		do {
#if HAVE_SCHED_H
			sched_yield();
#endif /* HAVE_SCHED_H */
			data = queue_deq(&ctx->source_queue, &type, NULL);
		} while (data == NULL);
		switch (type) {
		case CLIENT_MESSAGE_DATA:
			printf("client: data = 0x%"PRIx32"\n", *data);
			free(data);
			done = 1;
			break;
		default:
			ctx->ret = EINVAL;
			return &ctx->ret;
		}
	}
	/* hermes_create_msg(); */

	if ((sctx = calloc(1, sizeof(*sctx))) == NULL) {
		ctx->ret = errno;
		return &ctx->ret;
	}
	if ((hctx = hermes_init(0, 0)) == NULL) {
		ctx->ret = errno;
		return &ctx->ret;
	}
	sctx->hctx = hctx;
	if ((node = hnode_create(sctx)) == NULL) {
		free(sctx);
		ctx->ret = 1;
		return &ctx->ret;
	}
	hash_insert(ctx->registry, node, &hctx->id, hctx->size);
	hermes_connect(hctx);
	hermes_send(hctx, NULL);
	hermes_disconnect(hctx);
	hermes_fini(hctx);

	return &ctx->ret;
}
