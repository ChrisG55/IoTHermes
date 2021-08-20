#include "client.h"
#include "config.h"
#include "format.h"
#include "hash.h"
#include "queue.h"
#include "queue_data.h"
#include "streamid.h"

#include <assert.h>
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
	struct format *format;
};

static int client_init(struct client_context *ctx)
{
	ctx->registry = hash_create(UINT_MAX, streamid_compare, NULL);
	if (ctx->registry == NULL)
		return 1;

	return 0;
}

static int message_init(hash_t *registry, void *msg, struct node *qnode)
{
	int rc = 0;
	hnode_t *hnode = NULL;
	struct client_msg_init *cmi;
	struct client_msg_init_response *response;
	struct source_context *sctx;

	cmi = msg;
	if ((sctx = calloc(1, sizeof(*sctx))) == NULL) {
		rc = errno;
		goto init_response;
	}
	sctx->queue = cmi->queue;
	if ((sctx->format = cmi->format) == NULL) {
		rc = 1;
		goto init_response;
	}
	if ((sctx->hctx = hermes_init(0, 0)) == NULL) {
		rc = 1;
		goto init_response;
	}
	if ((hnode = hnode_create(sctx)) == NULL) {
		free(sctx);
		rc = 1;
		goto init_response;
	}
	hash_insert(registry, hnode, &sctx->hctx->id, sctx->hctx->size);
init_response:
	assert(sizeof(struct client_msg_init) >= sizeof(struct client_msg_init_response));
	response = msg;
	response->return_value = rc;
	if (hnode) {
		response->id = sctx->hctx->id;
		response->size = sctx->hctx->size;
	}
	rc = queue_enq(cmi->queue, response, CLIENT_MESSAGE_RESPONSE, qnode);
	if (rc != 0) {
		free(msg);
		free(qnode);
	}

	return rc;
}

void *client_main(void *c)
{
	struct client_context *ctx = c;
	uint32_t *data;
	void *msg;
	unsigned char type;
	int done = 0;
	struct hermes_context *hctx = NULL;
	struct node *node = NULL;
	int rc;

	if ((ctx->ret = client_init(ctx)) != 0)
		return &ctx->ret;

	while (!done) {
		do {
#if HAVE_SCHED_H
			sched_yield();
#endif /* HAVE_SCHED_H */
			msg = queue_deq(&ctx->source_queue, &type, node);
		} while (msg == NULL);
		switch (type) {
		case CLIENT_MESSAGE_DATA:
			data = msg;
			printf("client: data = 0x%"PRIx32"\n", *data);
			free(data);
			done = 1;
			break;
		case CLIENT_MESSAGE_INIT:
			rc = message_init(ctx->registry, msg, node);
			if (rc != 0) {
				ctx->ret = rc;
				return &ctx->ret;
			}
			break;
		default:
			ctx->ret = EINVAL;
			return &ctx->ret;
		}
	}
	/* hermes_create_msg(); */

	hermes_connect(hctx);
	hermes_send(hctx, NULL);
	hermes_disconnect(hctx);
	hermes_fini(hctx);

	return &ctx->ret;
}
