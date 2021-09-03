#include "client.h"
#include "config.h"
#include "format.h"
#include "hash.h"
#include "io.h"
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

static int message_data(const hash_t *registry, void *msg, struct node *qnode)
{
	/* uint32_t *val; */
	struct client_msg_data *cmd = msg;
	hnode_t *hnode;
	struct source_context *sctx;
	int *response;
	int rc = 0;

	/* val = cmd->data; */
	/* printf("client: data = 0x%"PRIx32"\n", *val); */
	printf("client: data = 0x%x\n", *(int *)cmd->data);
	hnode = hash_lookup(registry, &cmd->id, cmd->size);
	/*
	 * NOTE: this is a message protocol violation where either the source
	 * did not sent a client init message first or the client init message
	 * was never received by the client. Thus, the client has no means to
	 * contact the source about the issue. The client will
	 * 1) print an error message
	 * 2) delete the message to free resources (ignore the message)
	 * 3) return a success return code
	 * Since the client is purely reactive, it is up to the source to figure
	 * out the issue.
	 */
	if (hnode == NULL) {
		gdiot_fprintf(stderr, "Client: message protocol violation. Received data message for unknown ID %lu and size %z. Source might be stuck in infinite loop.\n", cmd->id, cmd->size);
		free(cmd->data);
		free(cmd);
		return rc;
	}
	sctx = hnode->hash_data;
	free(msg);
	if ((response = calloc(1, sizeof(*response))) == NULL) {
		free(qnode);
		return 1;
	}
	*response = rc;
	rc = queue_enq(sctx->queue, response, CLIENT_MESSAGE_RESPONSE, qnode);
	if (rc != 0) {
		free(response);
		free(qnode);
	}

	return rc;
}

static int message_fini(hash_t *registry, void *msg, struct node *qnode)
{
	int rc = 0;
	struct client_msg_fini *cmf;
	hnode_t *hnode;
	struct source_context *sctx;
	int *response;

	cmf = msg;
	gdiot_printf("client %lu: fini\n", cmf->id);
	hnode = hash_lookup(registry, &cmf->id, cmf->size);
	if (hnode == NULL) {
		rc = 1;
		goto fini_end;
	}
	hnode = hash_delete(registry, hnode);
	sctx = hnode->hash_data;
	if (hermes_fini(sctx->hctx) == -1)
		rc = errno;
	if ((response = calloc(1, sizeof(*response))) == NULL) {
		rc = 1;
		goto fini_noresponse;
	}
	*response = rc;
	rc = queue_enq(sctx->queue, response, CLIENT_MESSAGE_RESPONSE, qnode);
	if (rc != 0)
		free(response);

fini_noresponse:
	free(sctx);
	free(hnode);
	if (rc != 0)
		free(qnode);
fini_end:
	free(msg);

	return rc;
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
	sctx->hctx = hermes_init(&cmi->hints, cmi->nodename, cmi->port, 0, 0,
				 HERMES_CLIENT);
	if (sctx->hctx == NULL) {
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
	void *msg;
	unsigned char type;
	int done = 0;
	/* struct hermes_context *hctx = NULL; */
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
			rc = message_data(ctx->registry, msg, node);
			if (rc != 0) {
				ctx->ret = rc;
				return &ctx->ret;
			}
			break;
		case CLIENT_MESSAGE_FINI:
			rc = message_fini(ctx->registry, msg, node);
			if (rc != 0) {
				ctx->ret = rc;
				return &ctx->ret;
			}
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

	/* NOTE: comment for now, uncomment once implemented */
	/* hermes_connect(hctx); */
	/* hermes_send(hctx, NULL); */
	/* hermes_disconnect(hctx); */

	return &ctx->ret;
}
