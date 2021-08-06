#include "hash.h"
#include "queue.h"
#include "queue_data.h"
#include "server.h"
#include "streamid.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "lib/hermes.h"

struct client_context {
	int fd;
};

static int server_init(struct server_context *ctx)
{
	ctx->registry = hash_create(UINT_MAX, streamid_compare, NULL);
	if (ctx->registry == NULL)
		return 1;

	return 0;
}

void *server_main(void *c)
{
	struct server_context *ctx = c;
	uint32_t *data;

	if ((ctx->ret = server_init(ctx)) != 0)
		return &ctx->ret;

	ctx->hermes = hermes_init(&ctx->hints, ctx->nodename, ctx->port, 0, 0,
				  HERMES_SERVER);
	if (ctx->hermes == NULL) {
		ctx->ret = errno;
		return &ctx->ret;
	}
	hermes_connect(ctx->hermes);
	hermes_send(ctx->hermes, NULL);
	hermes_disconnect(ctx->hermes);
	if (hermes_fini(ctx->hermes) == -1) {
		ctx->ret = errno;
		return &ctx->ret;
	}

	if ((data = calloc(1, sizeof(*data))) == NULL) {
		ctx->ret = errno;
		return &ctx->ret;
	}
	*data = 0xc0ffee00;

	ctx->ret = queue_enq(ctx->queue, data, CLIENT_MESSAGE_DATA, NULL);

	return &ctx->ret;
}
