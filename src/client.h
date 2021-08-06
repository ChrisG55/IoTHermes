#ifndef CLIENT_H
#define CLIENT_H

#include "config.h"
#include "hash.h"
#include "queue.h"

#include "lib/hermes.h"

#if HAVE_NETDB_H
#include <netdb.h>
#else
#error "system does not support addrinfo structure"
#endif

struct client_context {
	int ret;
	hash_t *registry;
	struct queue source_queue;
	struct addrinfo hints;
	const char *nodename;
};

void *client_main(void *c);

#endif /* CLIENT_H */
