/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef SOURCE_H
#define SOURCE_H

#include "config.h"
#include "format.h"
#include "queue.h"

#include <stdio.h>

#if HAVE_CSV_H
#include <csv.h>
#endif /* HAVE_CSV_H */

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
	/* CSV runtime variables */
	unsigned int field;
	unsigned int row;
	void *data;
	char *buf;
	FILE *fp;
#if HAVE_CSV_H
	struct csv_parser *p;
#endif /* HAVE_CSV_H */
	/* initialization or runtime variables */
	unsigned long id;
	size_t id_size;
	/* initialization variables */
	enum source_type type;
	int fd;
	struct queue *queue;
	struct queue *client_queue;
	struct format format;
};

void *source_main(void *c);

#endif /* SOURCE_H */
