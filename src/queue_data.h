#ifndef QUEUE_DATA_H
#define QUEUE_DATA_H

#include "config.h"
#include "format.h"
#include "queue.h"

#if HAVE_NETDB_H
#include <netdb.h>
#else
#error "system does not support addrinfo structure"
#endif

enum queue_data {
	CLIENT_MESSAGE_DATA,
	CLIENT_MESSAGE_INIT,
	CLIENT_MESSAGE_RESPONSE,
};

struct client_msg_init {
	const char *nodename;
	const char *port;
	struct addrinfo hints;
	unsigned long id;
	struct queue *queue;
	struct format *format;
};

#endif /* QUEUE_DATA_H */
