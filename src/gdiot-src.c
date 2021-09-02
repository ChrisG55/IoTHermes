/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT source.
 */
#include "args.h"
#include "client.h"
#include "config.h"
#include "exit.h"
#include "gdiot-src.h"
#include "io.h"
#include "queue.h"
#include "queue_data.h"
#include "source.h"
#include "task.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

static const char *help_line = "GDIoT source node reference implementation";

struct source_context *sctx = NULL;
int nb_sources = 0;

int main(int argc, char *argv[])
{
	int rc;
	int rv = EXIT_SUCCESS;
	int *crc;
	task_t client, source;
	struct client_context cctx;
	uint32_t *data;
	struct queue queue;
	char *fn;

	conf.help_line = help_line;

	parse_args(argc, argv);

#if HAVE_UNISTD_H && (defined(_DEFAULT_SOURCE) || defined(_XOPEN_SOURCE))
	srand48(time(NULL));
#else
	srand(time(NULL));
#endif /* HAVE_UNISTD_H && (_DEFAULT_SOURCE || _XOPEN_SOURCE) */

	if (queue_init(&queue) != 0)
		return errno2exit();

	if ((rc = queue_init(&cctx.source_queue)) != 0)
		return errno2exit();

	if ((rc = task_create(client_main, &client, (void *)&cctx)) != 0) {
		errno = rc;
		return errno2exit();
	}

	if ((sctx = calloc(1, sizeof(*sctx))) == NULL)
		return errno2exit();
	nb_sources++;
	if ((fn = calloc(9, sizeof(*fn))) == NULL)
		return errno2exit();
	fn[0] = 't';
	fn[1] = 'e';
	fn[2] = 's';
	fn[3] = 't';
	fn[4] = '.';
	fn[5] = 'c';
	fn[6] = 's';
	fn[7] = 'v';
	fn[8] = '\0';
	sctx[0].type = SOURCE_CSV;
#if HAVE_FCNTL_H && HAVE_SYS_STAT_H
	sctx[0].fd = open(fn, O_RDONLY);
#endif
	sctx[0].queue = &queue;
	sctx[0].client_queue = &cctx.source_queue;

	if ((rc = task_create(source_main, &source, (void *)&sctx[0])) != 0) {
		errno = rc;
		return errno2exit();
	}

	if ((rc = task_join(source, (void **)&crc)) != 0) {
		errno = rc;
		rv = errno2exit();
	} else if (*crc != 0) {
		rv = EXIT_FAILURE;
	}

	if ((data = calloc(1, sizeof(*data))) == NULL)
		return errno2exit();
	*data = 0xc0ffee00;
	rc = queue_enq(&cctx.source_queue, data, CLIENT_MESSAGE_DATA, NULL);
	if (rc != 0) {
		free(data);
		return errno2exit();
	}

	if ((rc = task_join(client, (void **)&crc)) != 0) {
		errno = rc;
		rv = errno2exit();
	} else if (*crc != 0) {
		rv = EXIT_FAILURE;
	}

	task_free(client);
	task_free(source);

	free(fn);

	return rv;
}
