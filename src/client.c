#include "client.h"

#include <stdlib.h>

#include "lib/hermes.h"

void *client_main(void *c)
{
	struct client_context *ctx = c;

	ctx->ret = 0;

	ctx->hermes = hermes_init();
	hermes_connect(ctx->hermes);
	hermes_send(ctx->hermes, NULL);
	hermes_disconnect(ctx->hermes);
	hermes_fini(ctx->hermes);

	return &ctx->ret;
}
