/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "config.h"
#include "csv.h"
#include "format.h"
#include "io.h"
#include "source.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_CSV_H
#include <csv.h>
#endif /* HAVE_CSV_H */

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#if HAVE_CSV_H
static void field_cb(void *str, size_t len, void *v)
{
	struct source_context *ctx;
	unsigned int field;
	void *data;
	struct element *elements;
	size_t offset, size;
	union {
		char *c;
		unsigned char *uc;
		short *s;
		unsigned short *us;
		int *i;
		unsigned int *ui;
		long *l;
		unsigned long *ul;
		long long *ll;
		unsigned long long *ull;
		float *f;
		double *d;
		long double *ld;
	} u;

	ctx = v;
	field = ctx->field;
	data = ctx->data;
	elements = ctx->format.elements;
	size = ctx->format.size;

	if (field > size) {
		ctx->error = ERANGE;
		csv_fini(ctx->p, NULL, NULL, NULL);
	}

	offset = field == 0 ? 0 : elements[field - 1].offset;
	switch (elements[field].type) {
	case ELEMENT_CHAR:
		u.c = data;
		u.c += offset;
		*u.c = strtol(str, NULL, 10);
		break;
	case ELEMENT_UCHAR:
		u.uc = data;
		u.uc += offset;
		*u.uc = strtoul(str, NULL, 10);
		break;
	case ELEMENT_UCHAR8:
		u.uc = data;
		u.uc += offset;
		*u.uc = strtoul(str, NULL, 8);
		break;
	case ELEMENT_UCHAR16:
		u.uc = data;
		u.uc += offset;
		*u.uc = strtoul(str, NULL, 16);
		break;
	case ELEMENT_SHORT:
		u.s = data;
		u.s += offset;
		*u.s = strtol(str, NULL, 10);
		break;
	case ELEMENT_USHORT:
		u.us = data;
		u.us += offset;
		*u.us = strtoul(str, NULL, 10);
		break;
	case ELEMENT_USHORT8:
		u.us = data;
		u.us += offset;
		*u.us = strtoul(str, NULL, 8);
		break;
	case ELEMENT_USHORT16:
		u.us = data;
		u.us += offset;
		*u.us = strtoul(str, NULL, 16);
		break;
	case ELEMENT_INT:
		u.i = data;
		u.i += offset;
		*u.i = strtol(str, NULL, 10);
		break;
	case ELEMENT_UINT:
		u.ui = data;
		u.ui += offset;
		*u.ui = strtoul(str, NULL, 10);
		break;
	case ELEMENT_UINT8:
		u.ui = data;
		u.ui += offset;
		*u.ui = strtoul(str, NULL, 8);
		break;
	case ELEMENT_UINT16:
		u.ui = data;
		u.ui += offset;
		*u.ui = strtoul(str, NULL, 16);
		break;
	case ELEMENT_LONG:
		u.l = data;
		u.l += offset;
		*u.l = strtol(str, NULL, 10);
		break;
	case ELEMENT_ULONG:
		u.ul = data;
		u.ul += offset;
		*u.ul = strtoul(str, NULL, 10);
		break;
	case ELEMENT_ULONG8:
		u.ul = data;
		u.ul += offset;
		*u.ul = strtoul(str, NULL, 8);
		break;
	case ELEMENT_ULONG16:
		u.ul = data;
		u.ul += offset;
		*u.ul = strtoul(str, NULL, 16);
		break;
	case ELEMENT_LLONG:
		u.ll = data;
		u.ll += offset;
		*u.ll = strtoll(str, NULL, 10);
		break;
	case ELEMENT_ULLONG:
		u.ull = data;
		u.ull += offset;
		*u.ull = strtoull(str, NULL, 10);
		break;
	case ELEMENT_ULLONG8:
		u.ull = data;
		u.ull += offset;
		*u.ull = strtoull(str, NULL, 8);
		break;
	case ELEMENT_ULLONG16:
		u.ull = data;
		u.ull += offset;
		*u.ull = strtoull(str, NULL, 16);
		break;
	case ELEMENT_FLOAT:
		u.f = data;
		u.f += offset;
		*u.f = strtof(str, NULL);
		break;
	case ELEMENT_DOUBLE:
		u.d = data;
		u.d += offset;
		*u.d = strtod(str, NULL);
		break;
	case ELEMENT_LDOUBLE:
		u.ld = data;
		u.ld += offset;
		*u.ld = strtold(str, NULL);
		break;
	default:
		ctx->error = EINVAL;
		csv_fini(ctx->p, NULL, NULL, NULL);
	}

	ctx->field = ++field;
}

static size_t fill_buffer(char *buf, FILE *fp, int *error, size_t nbyte)
{
	int nblock; /* non-blocking file access */
	size_t bytes_read, bytes_to_read;
	size_t total_bytes;

	total_bytes = nbyte;

	if (nbyte > SSIZE_MAX)
		gdiot_fprintf(stderr,
			      "WARNING: %lu (nbtye) > %ld (SSIZE_MAX)\nThe result of read() is implementation-defined\n",
			      nbyte, SSIZE_MAX);

	nblock = fcntl(fileno(fp), F_GETFL) & O_NONBLOCK;

	for (; nbyte > 0; nbyte -= bytes_read, buf += bytes_read) {
		int ferr;
		bytes_to_read = nbyte > (BUFSIZ - 1) ? (BUFSIZ - 1) : nbyte;

		bytes_read = fread(buf, sizeof(*buf), bytes_to_read, fp);
		ferr = ferror(fp);
		if (nblock && ferr && errno == EAGAIN) {
			bytes_read = 0;
			continue; /* EAGAIN */
		} else if (feof(fp) && bytes_read <= bytes_to_read) {
			nbyte -= bytes_read;
			break; /* EOF */
		} else if (ferr && bytes_read < bytes_to_read &&
			   (errno == EBADF ||
			    errno == EINTR ||
			    errno == EIO   ||
			    errno == EOVERFLOW)) {
			*error = errno;
			return 0;
		}
		buf[bytes_read] = '\0';
	}

	return total_bytes - nbyte;
}

/*
 * Find the end of the record in the buffer pointed to by buf. Returns a pointer
 * to then end of the record or the end of the last datum in the buffer. A NULL
 * pointer only if neither the end of a record nor the end of a datum can be
 * found.
 */
static char *find_record_end(const char *buf, char delim, int eof)
{
	char *record_end;

	record_end = strchr(buf, '\n');
	/* Does the file not end in a newline at EOF? */
	if ((record_end == NULL) && eof)
		record_end = strchr(buf, '\0');
	/* Record is larger than buffer, consider last complete datum? */
	else if ((record_end == NULL) && !eof)
		record_end = strrchr(buf, delim);
	/* Check return value for datum larger than buffer */
	return record_end;
}

static void row_cb(int c, void *v)
{
	struct source_context *ctx;
	ctx = v;
	ctx->row++;
	ctx->field = 0;
}

static void csv_error2errno(int code)
{
	switch (code) {
	case CSV_EPARSE:
		errno = EFAULT; /* NOTE: strict mode error */
		break;
	case CSV_ENOMEM:
		errno = ENOMEM;
		break;
	case CSV_ETOOBIG:
		errno = E2BIG; /* NOTE: buffer > SIZE_MAX */
		break;
	case CSV_EINVALID:
		errno = EINVAL;
		break;
	default:
		errno = code;
	}
}
#endif /* HAVE_CSV_H */

int csv_finish(struct source_context *ctx)
{
	if (ctx->buf)
		free(ctx->buf);

#if HAVE_CSV_H
	csv_fini(ctx->p, NULL, NULL, NULL);
	csv_free(ctx->p);
	free(ctx->p);
#endif /* HAVE_CSV_H */

	fclose(ctx->fp);

	return 0;
}

int csv_initialize(struct source_context *ctx, size_t len)
{
#if HAVE_CSV_H
	if ((ctx->p = calloc(1, sizeof(*ctx->p))) == NULL)
		return 1;
	if (csv_init(ctx->p, CSV_STRICT | CSV_APPEND_NULL) != 0)
		goto csv_init_parser_err;
	if (ctx->type == SOURCE_TSV)
		csv_set_delim(ctx->p, CSV_TAB);
#endif /* HAVE_CSV_H */

	if (!(ctx->fp = fdopen(ctx->fd, "r")))
		goto csv_init_err;

	if ((ctx->buf = calloc(len, sizeof(*ctx->buf))) == NULL)
		goto csv_init_err;

	return 0;

csv_init_err:
	fclose(ctx->fp);
#if HAVE_CSV_H
csv_init_parser_err:
	csv_free(ctx->p);
	free(ctx->p);
#endif /* HAVE_CSV_H */
	return 1;
}

void *csv_data(struct source_context *ctx)
{
	char delim;
	char *buf, *buf_base, *record_end;
	int eof, record_complete;
	void *data;
	size_t bytes_read, bytes_to_read;

	delim = ctx->type == SOURCE_CSV ? ',' : '\t';
	buf_base = ctx->buf;
	eof = feof(ctx->fp);
	record_complete = 0;
	if ((data = ctx->data) == NULL) {
		data = calloc(1,
			      ctx->format.elements[ctx->format.size-1].offset);
		if (data == NULL) {
			ctx->error = errno;
			return data;
		}
		ctx->data = data;
	}

	while (!record_complete) {
		/* Fill buffer */
		buf = strchr(buf_base, '\0');
		bytes_to_read = &buf_base[BUFSIZ-2] - buf;
		if (!eof && bytes_to_read) {
			bytes_read = fill_buffer(buf, ctx->fp, &ctx->error,
						 bytes_to_read);
			if (bytes_read == 0 && ctx->error)
				goto err;
			eof = feof(ctx->fp);
		}
		buf = buf_base;
		/* Is datum larger than buffer? */
		record_end = find_record_end(buf, delim, eof);
		if (record_end == NULL) {
			ctx->error = E2BIG;
			goto err;
		}

		/* Consume record */
		assert((record_end != NULL) && (buf <= record_end));
#if HAVE_CSV_H
		bytes_to_read = record_end - buf + 1;
		bytes_read = csv_parse(ctx->p, buf, bytes_to_read, field_cb,
				       row_cb, ctx);
		if (bytes_read != bytes_to_read) {
			csv_error2errno(csv_error(ctx->p));
			goto err;
		}
#endif /* HAVE_CSV_H */
		if (*record_end == '\n') {
			record_complete = 1;
			buf = record_end + 1;
		} else if ((*record_end == '\0') && eof) {
			record_complete = 1;
			buf = record_end;
		}

		/* Move buffer */
		if ((buf != buf_base) && !eof) {
			size_t diff;
			diff = strchr(buf, '\0') - buf;
			buf = memmove(buf_base, buf, (diff + 1) * sizeof(*buf));
		}
	}

	ctx->data = NULL;
	return data;

err:
	free(data);
	ctx->data = NULL;
	return NULL;
}
