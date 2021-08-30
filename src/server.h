#ifndef SERVER_H
#define sERVER_H

#include "hash.h"
#include "queue.h"

#include "lib/hermes.h"

struct server_context {
	int ret;
	struct hermes_context *hermes;
	struct queue *queue;
	hash_t *registry;
};

void *server_main(void *c);

#endif /* SERVER_H */
