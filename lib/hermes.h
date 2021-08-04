#ifndef HERMES_H
#define HERMES_H

struct hermes_context {
	unsigned long id;
};

void hermes_connect(struct hermes_context *ctx);
void hermes_disconnect(struct hermes_context *ctx);
void hermes_fini(struct hermes_context *ctx);
struct hermes_context *hermes_init(unsigned long id);
void hermes_send(struct hermes_context *ctx, void *data);

#endif /* HERMES_H */
