#ifndef HERMES_H
#define HERMES_H

#include <stddef.h>

#define HERMES_SERVER_PORT_LEN 6
#define HERMES_SERVER_PORT_MIN 49152U
#define HERMES_SERVER_PORT_MAX 65535U

#define HERMES_DEFAULT_STREAM_ID_SIZE 1

struct hermes_context {
	size_t size;
	unsigned long id;
};

void hermes_connect(struct hermes_context *ctx);
void hermes_disconnect(struct hermes_context *ctx);
void hermes_fini(struct hermes_context *ctx);
struct hermes_context *hermes_init(unsigned long id, size_t s);
void hermes_send(struct hermes_context *ctx, void *data);

#endif /* HERMES_H */
