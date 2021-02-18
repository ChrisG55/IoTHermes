#ifndef SERVER_H
#define sERVER_H

#include "queue.h"

#include "lib/hermes.h"

struct server_context {
	int ret;
	struct hermes_context *hermes;
	struct queue *queue;
};

void *server_main(void *c);

#endif /* SERVER_H */
