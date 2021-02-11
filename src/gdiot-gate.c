/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Generalized deduplication IoT gateway.
 */
#include "args.h"

#include <stdlib.h>

static const char *help_line = "GDIoT intermediate node reference implementation";

int main(int argc, char *argv[])
{
	conf.help_line = help_line;

	parse_args(argc, argv);

	return EXIT_SUCCESS;
}
