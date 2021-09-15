/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "format.h"
#include "io.h"

#include <errno.h>
#include <stddef.h>

/*
 * Compute the size in bytes for an element formated in data. Upon successful
 * computation returns a non-zero value as the size of the element, otherwise, a
 * size of zero is returned and errno is set to EINVAL to indicate that idx is
 * out of range.
 */
size_t format_element_size(struct format *f, int idx)
{
	size_t predoff;

	if ((idx < 0) || (f->size < idx)) {
		errno = EINVAL;
		return 0;
	}

	predoff = idx == 0 ? 0 : f->elements[idx-1].offset;
	return f->elements[idx].offset - predoff;
}

int format_print(const struct format *f)
{
	int i, r, rv;

	if ((rv = gdiot_puts("Format:")) == EOF)
		return rv;
	if ((r = gdiot_printf("- size: %z\n", f->size)) < 0)
		return r;
	rv += r;
	if ((r = gdiot_printf("- elements:\n")) < 0)
		return r;
	rv += r;
	for (i = 0; i < f->size; i++) {
		r = gdiot_printf("  + %d: %d %z\n", i, f->elements[i].type,
				 f->elements[i].offset);
		if (r < 0)
			return r;
		rv += r;
	}
	if ((r = gdiot_printf("\n")) < 0)
		return r;

	return rv + r;
}
