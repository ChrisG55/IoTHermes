#include "queue.h"
#include "server.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "lib/hermes.h"

void *server_main(void *c)
{
	struct server_context *ctx = c;
	uint32_t *data;

	if ((ctx->hermes = hermes_init(0)) == NULL) {
		ctx->ret = errno;
		return &ctx->ret;
	}
	hermes_connect(ctx->hermes);
	hermes_send(ctx->hermes, NULL);
	hermes_disconnect(ctx->hermes);
	hermes_fini(ctx->hermes);

	if ((data = calloc(1, sizeof(*data))) == NULL) {
		ctx->ret = errno;
		return &ctx->ret;
	}
	*data = 0xc0ffee00;

	ctx->ret = queue_enq(ctx->queue, data);

	return &ctx->ret;
}
