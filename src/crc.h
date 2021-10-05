/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CRC_H
#define CRC_H

#include "hash.h"

#include <stddef.h>

hash_val_t crc32(const void *buf, size_t len);

#endif /* CRC_H */
