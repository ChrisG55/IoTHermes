/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Code modified from Rosetta Code
 * https://rosettacode.org/wiki/CRC-32#Implementation_2
 */
#include "crc.h"
#include "hash.h"

#include <stddef.h>
#include <stdint.h>

hash_val_t crc32(const void *buf, size_t len)
{
	static uint32_t table[256];
	static int table_initialized = 0;
	uint32_t rem;
	uint8_t octet;
	int i, j;
	const uint8_t *p; /* start pointer */
	const uint8_t *q; /* end pointer */
	uint32_t crc = UINT32_MAX;

	/* This check is not thread safe; there is no mutex. */
	if (table_initialized == 0) {
		/* Calculate CRC table. */
		for (i = 0; i < 256; i++) {
			rem = i;  /* remainder from polynomial division */
			for (j = 0; j < 8; j++) {
				if (rem & 1) {
					rem >>= 1;
					rem ^= 0xedb88320;
				} else
					rem >>= 1;
			}
			table[i] = rem;
		}
		table_initialized = 1;
	}

	q = buf;
	q += len;
	for (p = buf; p < q; p++) {
		octet = *p;  /* Cast to unsigned octet. */
		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
	}
	return ~crc;
}
