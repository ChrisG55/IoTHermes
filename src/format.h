/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef FORMAT_H
#define FORMAT_H

#include <stddef.h>

enum element_type {
	ELEMENT_CHAR,
	ELEMENT_UCHAR,
	ELEMENT_UCHAR8,
	ELEMENT_UCHAR16,
	ELEMENT_SHORT,
	ELEMENT_USHORT,
	ELEMENT_USHORT8,
	ELEMENT_USHORT16,
	ELEMENT_INT,
	ELEMENT_UINT,
	ELEMENT_UINT8,
	ELEMENT_UINT16,
	ELEMENT_LONG,
	ELEMENT_ULONG,
	ELEMENT_ULONG8,
	ELEMENT_ULONG16,
	ELEMENT_LLONG,
	ELEMENT_ULLONG,
	ELEMENT_ULLONG8,
	ELEMENT_ULLONG16,
	ELEMENT_FLOAT,
	ELEMENT_DOUBLE,
	ELEMENT_LDOUBLE,
	ELEMENT_STRING,
};

struct element {
	enum element_type type;
	size_t offset;
};

/*
 * Defines the data layout in byte offsets. The first element always starts at
 * offset zero. The first offset in the format describes the second data
 * element! Therefore, in order to determine the size of a given element within
 * the data, one can calculate the difference of offsets to its
 * predecessor element. E.g., (notice that idx starts at zero)
 *
 * size_t element_size(struct format *f, int idx) {
 *         size_t predoff;
 *         predoff = idx == 0 ? 0 : s->offset[idx-1];
 *         return s->offset[idx] - predoff;
 * }
 *
 * size_t se2 = element_size(s, 1);
 *
 * It can be used to describe either sizes of values or sizes of transformed
 * values such as after generalized deduplication. Without this format
 * information, there is no way to make any sense out of the data it describes.
 */
struct format {
	size_t size;
	struct element *elements;
};

size_t format_element_size(struct format *f, int idx);
int format_print(const struct format *f);

#endif /* FORMAT_H */
