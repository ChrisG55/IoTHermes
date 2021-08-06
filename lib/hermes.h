#ifndef HERMES_H
#define HERMES_H

#include "config.h"

#include <stddef.h>

#if HAVE_NETDB_H
#include <netdb.h>
#else
#error "system does not support addrinfo structure"
#endif

#define HERMES_CLIENT 0
#define HERMES_SERVER 1

#define HERMES_SERVER_PORT_LEN 6
#define HERMES_SERVER_PORT_MIN 49152U
#define HERMES_SERVER_PORT_MAX 65535U

#define HERMES_DEFAULT_STREAM_ID_SIZE 1

struct hermes_context {
	size_t size;
	unsigned long id;
	int sfd;
};

void hermes_connect(struct hermes_context *ctx);
void hermes_disconnect(struct hermes_context *ctx);
int hermes_fini(struct hermes_context *ctx);
struct hermes_context *hermes_init(const struct addrinfo *restrict hints,
				   const char *restrict nodename,
				   const char *restrict port,
				   unsigned long id,
				   size_t s,
				   int type);
void hermes_send(struct hermes_context *ctx, void *data);

#endif /* HERMES_H */
