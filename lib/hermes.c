#include "config.h"
#include "hermes.h"

#include <limits.h>
#include <stdlib.h>

/*
 * Return a random stream identification.
 */
static unsigned long random_id(void)
{
	unsigned long id;

#if HAVE_UNISTD_H && (defined(_DEFAULT_SOURCE) || defined(_XOPEN_SOURCE))
	if (4294967295ULL < ULONG_MAX) {
		int i;
		int n = (int)(ULONG_MAX / 4294967295ULL) + 1;

		for (i = 0; i <= n; i++)
			id += mrand48();
	} else {
		id = mrand48();
	}
#else
	if (RAND_MAX > ULONG_MAX) {
		id = rand() % ULONG_MAX;
	} else if (RAND_MAX < ULONG_MAX) {
		int i;
		int n = (int)(ULONG_MAX / RAND_MAX) + 1;

		for (i = 0; i <= n; i++)
			id += rand();
	} else {
		id = rand();
	}
#endif /* HAVE_UNISTD_H && (_DEFAULT_SOURCE || _XOPEN_SOURCE) */

	return id;
}

void hermes_connect(struct hermes_context *ctx)
{

}

void hermes_disconnect(struct hermes_context *ctx)
{

}

void hermes_fini(struct hermes_context *ctx)
{

}

struct hermes_context *hermes_init(unsigned long id)
{
	struct hermes_context *ctx;

	ctx = calloc(1, sizeof(*ctx));
	if (ctx == NULL)
		return ctx;

	if (id)
		ctx->id = id;
	else
		ctx->id = random_id();

	return ctx;
}

void hermes_send(struct hermes_context *ctx, void *data)
{

}
