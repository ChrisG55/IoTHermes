/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef CSV_H
#define CSV_H

#include "source.h"

#include <stddef.h>

int csv_finish(struct source_context *ctx);
int csv_initialize(struct source_context *ctx, size_t len);
void *csv_data(struct source_context *ctx);

#endif /* CSV_H */
