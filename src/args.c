/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "args.h"
#include "config.h"
#include "exit.h"
#include "format.h"
#include "io.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_GDIOT_SRC
#include "gdiot-src.h"
#elif HAVE_GDIOT_GATE
#include "gdiot-gate.h"
#elif HAVE_GDIOT_EVAP
#include "gdiot-evap.h"
#else
#error "implementation is not supported"
#endif /* HAVE_GDIOT_SRC */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

enum length_modifier {
	LMOD_NONE,
	LMOD_CHAR,
	LMOD_SHORT,
	LMOD_LONG,
	LMOD_LLONG,
	LMOD_FLOAT,
	LMOD_LDOUBLE,
};

struct config conf;

static int parse_token(const char *token, size_t offset, struct element *e);

static int create_source_context(void)
{
	if (nb_sources == 0)
		sctx = calloc(1, sizeof(*sctx));
	else
		sctx = realloc(sctx, nb_sources+1);
	if (sctx == NULL)
		return 1;
	nb_sources++;
	return 0;
}

static int parse_format(int idx, char *format)
{
	const char *separator = "%";
	char *state, *token;
	size_t offset = 0, size = 0;
	struct element *elements = NULL;

	if (idx == nb_sources) {
		if (create_source_context() != 0)
			return 1;
	}

	if ((token = strtok_r(format, separator, &state)) == NULL)
		return 1;
	while (token != NULL) {
		elements = realloc(elements, ++size * sizeof(*elements));
		if (elements == NULL)
			return 1;
		if (parse_token(token, offset, &elements[size - 1]) != 0)
			return 1;
		offset = elements[size - 1].offset;
		token = strtok_r(NULL, separator, &state);
	}

	sctx[idx].format.size = size;
	sctx[idx].format.elements = elements;

	return 0;
}

static int parse_token(const char *token, size_t offset, struct element *e)
{
	char *buf;
	const char *c, *wstart, *wend;
	int done, started;
	unsigned long width;
	size_t size;
	enum length_modifier lmod;
	enum element_type type;

	c = token;
	wstart = wend = NULL;
	done = 0;
	started = 0; /* first token character found? */
	size = 0;
	lmod = LMOD_NONE;

	while (!done) {
		switch (*c) {
		case 'h':
			/* parsing length modifiers */
			if ((started == 0) && (lmod == LMOD_NONE)) {
				started = 1;
			} else if ((started == 1) && (lmod == LMOD_NONE)) {
				errno = EINVAL;
				return 1;
			}
			if (lmod == LMOD_NONE) {
				lmod = LMOD_SHORT;
			} else if (lmod == LMOD_SHORT) {
				lmod = LMOD_CHAR;
			} else {
				errno = EINVAL;
				return 1;
			}
			break;
		case 'l':
			if ((started == 0) && (lmod == LMOD_NONE)) {
				started = 1;
			} else if ((started == 1) && (lmod == LMOD_NONE)) {
				errno = EINVAL;
				return 1;
			}
			if (lmod == LMOD_NONE) {
				lmod = LMOD_LONG;
			} else if (lmod == LMOD_LONG) {
				lmod = LMOD_LLONG;
			} else {
				errno = EINVAL;
				return 1;
			}
			break;
		case 'H':
			if ((started == 0) && (lmod == LMOD_NONE)) {
				started = 1;
			} else if ((started == 1) && (lmod == LMOD_NONE)) {
				errno = EINVAL;
				return 1;
			}
			if (lmod == LMOD_NONE) {
				lmod = LMOD_FLOAT;
			} else {
				errno = EINVAL;
				return 1;
			}
			break;
		case 'L':
			if ((started == 0) && (lmod == LMOD_NONE)) {
				started = 1;
			} else if ((started == 1) && (lmod == LMOD_NONE)) {
				errno = EINVAL;
				return 1;
			}
			if (lmod == LMOD_NONE) {
				lmod = LMOD_LDOUBLE;
			} else {
				errno = EINVAL;
				return 1;
			}
			break;
		case 'd':
		case 'i':
			/* parsing signed integer */
			if ((started == 0) && (lmod == LMOD_NONE)) {
				started = 1;
			} else if ((started == 0) && (lmod != LMOD_NONE))  {
				errno = EINVAL;
				return 1;
			}
			if (lmod == LMOD_CHAR) {
				size = sizeof(char);
				type = ELEMENT_CHAR;
			} else if (lmod == LMOD_SHORT) {
				size = sizeof(short);
				type = ELEMENT_SHORT;
			} else if (lmod == LMOD_NONE) {
				size = sizeof(int);
				type = ELEMENT_INT;
			} else if (lmod == LMOD_LONG) {
				size = sizeof(long);
				type = ELEMENT_LONG;
			} else if (lmod == LMOD_LLONG) {
				size = sizeof(long long);
				type = ELEMENT_LLONG;
			} else {
				errno = EINVAL;
				return 1;
			}
			done = 1;
			break;
		case 'o':
			/* parsing unsigned octal integer */
			if ((started == 0) && (lmod == LMOD_NONE)) {
				started = 1;
			} else if ((started == 0) && (lmod != LMOD_NONE))  {
				errno = EINVAL;
				return 1;
			}
			if (lmod == LMOD_CHAR) {
				size = sizeof(unsigned char);
				type = ELEMENT_UCHAR8;
			} else if (lmod == LMOD_SHORT) {
				size = sizeof(unsigned short);
				type = ELEMENT_USHORT8;
			} else if (lmod == LMOD_NONE) {
				size = sizeof(unsigned int);
				type = ELEMENT_UINT8;
			} else if (lmod == LMOD_LONG) {
				size = sizeof(long unsigned);
				type = ELEMENT_ULONG8;
			} else if (lmod == LMOD_LLONG) {
				size = sizeof(long long unsigned);
				type = ELEMENT_ULLONG8;
			} else {
				errno = EINVAL;
				return 1;
			}
			done = 1;
			break;
		case 'u':
			/* parsing unsigned integer */
			if ((started == 0) && (lmod == LMOD_NONE)) {
				started = 1;
			} else if ((started == 0) && (lmod != LMOD_NONE))  {
				errno = EINVAL;
				return 1;
			}
			if (lmod == LMOD_CHAR) {
				size = sizeof(unsigned char);
				type = ELEMENT_UCHAR;
			} else if (lmod == LMOD_SHORT) {
				size = sizeof(unsigned short);
				type = ELEMENT_USHORT;
			} else if (lmod == LMOD_NONE) {
				size = sizeof(unsigned int);
				type = ELEMENT_UINT;
			} else if (lmod == LMOD_LONG) {
				size = sizeof(long unsigned);
				type = ELEMENT_ULONG;
			} else if (lmod == LMOD_LLONG) {
				size = sizeof(long long unsigned);
				type = ELEMENT_ULLONG;
			} else {
				errno = EINVAL;
				return 1;
			}
			done = 1;
			break;
		case 'x':
		case 'X':
			/* parsing unsigned hexadecimal integer */
			if ((started == 0) && (lmod == LMOD_NONE)) {
				started = 1;
			} else if ((started == 0) && (lmod != LMOD_NONE))  {
				errno = EINVAL;
				return 1;
			}
			if (lmod == LMOD_CHAR) {
				size = sizeof(unsigned char);
				type = ELEMENT_UCHAR16;
			} else if (lmod == LMOD_SHORT) {
				size = sizeof(unsigned short);
				type = ELEMENT_USHORT16;
			} else if (lmod == LMOD_NONE) {
				size = sizeof(unsigned int);
				type = ELEMENT_UINT16;
			} else if (lmod == LMOD_LONG) {
				size = sizeof(long unsigned);
				type = ELEMENT_ULONG16;
			} else if (lmod == LMOD_LLONG) {
				size = sizeof(long long unsigned);
				type = ELEMENT_ULLONG16;
			} else {
				errno = EINVAL;
				return 1;
			}
			done = 1;
			break;
		case 'f':
		case 'F':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
		case 'a':
		case 'A':
			/* parsing double */
			if ((started == 0) && (lmod == LMOD_NONE)) {
				started = 1;
			} else if ((started == 1) && (lmod == LMOD_NONE)) {
				errno = EINVAL;
				return 1;
			}
			if (lmod == LMOD_FLOAT) {
				size = sizeof(float);
				type = ELEMENT_FLOAT;
			} else if (lmod == LMOD_NONE) {
				size = sizeof(double);
				type = ELEMENT_DOUBLE;
			} else if (lmod == LMOD_LDOUBLE) {
				size = sizeof(long double);
				type = ELEMENT_LDOUBLE;
			} else {
				errno = EINVAL;
				return 1;
			}
			done = 1;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (started == 0) {
				wstart = c;
				started = 1;
			}
			wend = c;
			break;
		case 's':
			/* parsing string, requires field width */
			if (((started == 0) && (lmod == LMOD_NONE) && (wstart == NULL) && (wend == NULL)) ||
			    ((started == 1) && (lmod != LMOD_NONE))) {
				errno = EINVAL;
				return 1;
			}
			if ((buf = calloc(21, sizeof(*buf))) == NULL)
				return 1;
			strncpy(buf, wstart, wend - wstart);
			width = strtoul(buf, NULL, 10);
			free(buf);
			size = width * sizeof(char);
			done = 1;
			break;
		default:
			/* consume any other characters */
			if (started == 1) {
				errno = EINVAL;
				return 1;
			}
		}
		c++;
	}

	e->type = type;
	e->offset = offset + size;

	return 0;
}

#if HAVE_UNISTD_H && (_XOPEN_VERSION >= 4 || defined(_XOPEN_XPG4) || defined(_XOPEN_XPG3) || defined(_XOPEN_XPG2))
void parse_args(int argc, char *argv[])
{
	int c;
	int errflg = 0;
	int rv;
	int format_idx = 0;
	int source_idx = 0;

	opterr = 0;
	while ((c = getopt(argc, argv, "f:h")) != -1) {
		switch (c) {
		case 'f':
			if ((rv = parse_format(format_idx++, optarg)) != 0)
				errflg++;
			break;
		case 'h':
			rv = usage(argv[0]);
			gdiot_exit(rv < 0);
		case ':':
			if (gdiot_fprintf(stderr,
					  "Option -%c requires an operand\n",
					  optopt) < 0)
				gdiot_exit(1);
			errflg++;
			break;
		case '?':
			if (gdiot_fprintf(stderr,
					  "Unrecognized option: '-%c'\n",
					  optopt) < 0)
				gdiot_exit(1);
			errflg++;
		}
	}
	if (errflg || (format_idx != source_idx)) {
		usage(argv[0]);
		errno = EINVAL;
		gdiot_exit(1);
	}
}
#else
#error "No implementation of parse_args() for this system"
#endif /* _XOPEN_VERSION || _XOPEN_XPG4 || _XOPEN_XPG3 || _XOPEN_XPG2 */

int usage(const char *progname)
{
	int r, rv;
	if (conf.help_line != NULL)
		if ((rv = gdiot_printf("%s\n", conf.help_line)) < 0)
			return rv;
	if ((r = gdiot_printf("Usage: %s [-f <fmt>] [-h]\n", progname)) < 0)
		return r;
	rv += r;
	if ((r = gdiot_puts("  -f   data format string")) == EOF)
	     return -1;
	rv += r;
	if ((r = gdiot_puts("  -h   help message")) == EOF)
		return -1;
	return rv + r;
}
