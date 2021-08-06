#ifndef SERVER_H
#define sERVER_H

#include "config.h"
#include "hash.h"
#include "queue.h"

#include "lib/hermes.h"

#if HAVE_NETDB_H
#include <netdb.h>
#else
#error "system does not support addrinfo structure"
#endif

struct server_context {
	int ret;
	struct hermes_context *hermes;
	struct queue *queue;
	hash_t *registry;
	struct addrinfo hints;
	const char *nodename;
	const char *port;
};

void *server_main(void *c);

#endif /* SERVER_H */
