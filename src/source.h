/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef SOURCE_H
#define SOURCE_H

#include "queue.h"

#include <stddef.h>

/* Type information for the sub-context, which can be CSV, I2C, GPIO or other */
enum source_type {
	SOURCE_CSV,
	SOURCE_TSV
};

struct source_context {
	/* runtime variables */
	int error;
	int ret;
	void *(*read)(struct source_context *);
	/* initialization or runtime variables */
	unsigned long id;
	size_t id_size;
	/* initialization variables */
	enum source_type type;
	int fd;
	struct queue *queue;
	struct queue *client_queue;
};

void *source_main(void *c);

#endif /* SOURCE_H */
