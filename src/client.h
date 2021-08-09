#ifndef CLIENT_H
#define CLIENT_H

#include "hash.h"
#include "queue.h"

#include "lib/hermes.h"

struct client_context {
	int ret;
	hash_t *registry;
	struct queue source_queue;
};

void *client_main(void *c);

#endif /* CLIENT_H */
