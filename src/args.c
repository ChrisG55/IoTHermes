/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "args.h"
#include "config.h"
#include "exit.h"
#include "io.h"

#include <errno.h>
#include <stdio.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

struct config conf;

#if HAVE_UNISTD_H && (_XOPEN_VERSION >= 4 || defined(_XOPEN_XPG4) || defined(_XOPEN_XPG3) || defined(_XOPEN_XPG2))
void parse_args(int argc, char *argv[])
{
	int c;
	int errflg = 0;
	int rv;

	opterr = 0;
	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
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
	if (errflg) {
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
	if ((r = gdiot_printf("Usage: %s [-h]\n", progname)) < 0)
		return r;
	rv += r;
	if ((r = gdiot_puts("  -h   help message")) == EOF)
		return -1;
	return rv + r;
}
