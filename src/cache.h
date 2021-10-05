/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CACHE_H
#define CACHE_H

#include "config.h"
#include "hash.h"

#include <limits.h>
#include <stddef.h>

#if HAVE_PTHREADS
#include <pthread.h>
#endif /* HAVE_PTHREADS */

struct cache {
	hash_t *hash;
#if HAVE_PTHREADS
	pthread_mutex_t lock;
#endif /* HAVE_PTHREADS */
};

int cache_delete(struct cache *cache, void *data, size_t len);
int cache_fini(struct cache *cache);
int cache_init(struct cache *cache);
int cache_insert(struct cache *cache, void *data, size_t len);
int cache_lookup(struct cache *cache, void *data, size_t len);

#endif /* CACHE_H */
