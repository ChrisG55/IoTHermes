#include "client.h"
#include "config.h"
#include "queue.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if HAVE_SCHED_H
#include <sched.h>
#endif /* HAVE_SCHED_H */

#include "lib/hermes.h"

void *client_main(void *c)
{
	struct client_context *ctx = c;
	uint32_t *data;
	unsigned char type;

	ctx->ret = 0;

	do {
#if HAVE_SCHED_H
		sched_yield();
#endif /* HAVE_SCHED_H */
		data = queue_deq(&ctx->source_queue, &type);
	} while (data == NULL);
	/* hermes_create_msg(); */
	printf("client: data = 0x%"PRIx32"\n", *data);
	free(data);

	if ((ctx->hermes = hermes_init(0, 0)) == NULL) {
		ctx->ret = errno;
		return &ctx->ret;
	}
	hermes_connect(ctx->hermes);
	hermes_send(ctx->hermes, NULL);
	hermes_disconnect(ctx->hermes);
	hermes_fini(ctx->hermes);

	return &ctx->ret;
}
