/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef ARGS_H
#define ARGS_H

struct block_code_config {
	int type;
};

struct config {
	const char *help_line;
	struct block_code_config block_code;
};

extern struct config conf;

/*
 * Print usage to standard output stream.
 * Returns a non-negative number if successfully executed, otherwise a negative
 * number is returned.
 * In a POSIX environment and only when an error occurred, errno shall be set
 * to indicate the error.
 */
int usage(const char *progname);
void parse_args(int argc, char *argv[]);

#endif /* ARGS_H */
