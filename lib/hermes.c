#include "config.h"
#include "hermes.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#if HAVE_NETDB_H
#include <netdb.h>
#else
#error "system does not support addrinfo structure"
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#else
#error "system does not support socket"
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

/*
 * Create a socket file descriptor and, if it is a HERMES_SERVER, bind a name to
 * it. Upon successful completion return the socket file descriptor, otherwise,
 * return -1 and set errno to indicate the error.
 */
static int bind_socket(const struct addrinfo *restrict hints,
		       const char *restrict nodename,
		       const char *restrict port,
		       int type)
{
	int ret, sfd;
	struct addrinfo *ai, *res;

	if ((ret = getaddrinfo(nodename, port, hints, &res)) != 0) {
		if (ret != EAI_SYSTEM)
			errno = ret;
		return -1;
	}

	for (ai = res; ai != NULL; ai = ai->ai_next) {
		sfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sfd == -1)
			continue;

		if (type == HERMES_SERVER) {
			if (bind(sfd, ai->ai_addr, ai->ai_addrlen) == 0)
				break;
		} else {
			break;
		}

		close(sfd);
	}

	freeaddrinfo(res);

	if (ai == NULL)
		return -1;

	return sfd;
}

/*
 * Return a random stream identification.
 * If the stream identification size s is 0, use the default size, otherwise,
 * truncate the stream identification size to s bytes. The stream identification
 * size s is ignored if it is greater than sizeof(unsigned long).
 */
static unsigned long random_id(size_t s)
{
	unsigned long id;

	if (s == 0)
		s = HERMES_DEFAULT_STREAM_ID_SIZE;

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

	if (s < sizeof(id))
		id &= ~(ULONG_MAX << (s * 8));

	return id;
}

void hermes_connect(struct hermes_context *ctx)
{

}

void hermes_disconnect(struct hermes_context *ctx)
{

}

int hermes_fini(struct hermes_context *ctx)
{
	int ret;
	int retry = 0;

	do {
		ret = close(ctx->sfd);
	} while (ret == -1 && errno == EINTR && retry++ < 8);
	free(ctx);

	return ret;
}

struct hermes_context *hermes_init(const struct addrinfo *restrict hints,
				   const char *restrict nodename,
				   const char *restrict port,
				   unsigned long id,
				   size_t s,
				   int type)
{
	struct hermes_context *ctx;

	ctx = calloc(1, sizeof(*ctx));
	if (ctx == NULL)
		return ctx;

	if (id)
		ctx->id = id;
	else
		ctx->id = random_id(s);

	if (s)
		ctx->size = s;
	else
		ctx->size = HERMES_DEFAULT_STREAM_ID_SIZE;

	if ((ctx->sfd = bind_socket(hints, nodename, port, type)) == -1) {
		free(ctx);
		ctx = NULL;
	}

	return ctx;
}

void hermes_send(struct hermes_context *ctx, void *data)
{

}
