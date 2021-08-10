#ifndef PEARSON_H
#define PEARSON_H

#include "hash.h"

#include <stdlib.h>

/* Pearson hash function for 8-bit hash values */
hash_val_t pearson8(const void *buf, size_t len);
/* Pearson hash function for 16-bit hash values */
hash_val_t pearson16(const void *buf, size_t len);

#endif /* PEARSON_H */
