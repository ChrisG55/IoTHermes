#ifndef CLIENT_H
#define CLIENT_H

#include "queue.h"

#include "lib/hermes.h"

struct client_context {
	int ret;
	struct hermes_context *hermes;
	struct queue source_queue;
};

void *client_main(void *c);

#endif /* CLIENT_H */
