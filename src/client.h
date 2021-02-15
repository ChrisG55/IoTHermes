#ifndef CLIENT_H
#define CLIENT_H

#include "lib/hermes.h"

struct client_context {
	int ret;
	struct hermes_context *hermes;
};

void *client_main(void *c);

#endif /* CLIENT_H */
