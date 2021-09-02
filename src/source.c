/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "source.h"

#include <errno.h>
#include <string.h>

static int source_fini(struct source_context *ctx)
{
	return 0;
}

static int source_init(struct source_context *ctx)
{
	switch (ctx->type) {
	case SOURCE_CSV:
	case SOURCE_TSV:
		break;
	default:
		return 1;
	}

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
