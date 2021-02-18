#ifndef SERVER_H
#define sERVER_H

#include "lib/hermes.h"

struct server_context {
	int ret;
	struct hermes_context *hermes;
};

void *server_main(void *c);

#endif /* SERVER_H */
