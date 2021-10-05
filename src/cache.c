/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "cache.h"
#include "config.h"
#include "crc.h"
#include "hash.h"

#include <stdlib.h>

#if HAVE_PTHREADS
#include <pthread.h>
#endif /* HAVE_PTHREADS */

int cache_delete(struct cache *cache, void *data, size_t len)
{
	hnode_t *hnode;

#if HAVE_PTHREADS
	if (pthread_mutex_lock(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */
	hnode = hash_lookup(cache->hash, data, len);
	if (hnode == NULL) {
#if HAVE_PTHREADS
		if (pthread_mutex_unlock(&cache->lock) != 0)
			return 1;
#endif /* HAVE_PTHREADS */
		return 0;
	}
	hnode = hash_delete(cache->hash, hnode);
#if HAVE_PTHREADS
	if (pthread_mutex_unlock(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */

	free(hnode->hash_data);
	free(hnode);

	return 0;
}

int cache_fini(struct cache *cache)
{
#if HAVE_PTHREADS
	if (pthread_mutex_lock(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */
	hash_destroy(cache->hash);
#if HAVE_PTHREADS
	if (pthread_mutex_unlock(&cache->lock) != 0)
		return 1;
	if (pthread_mutex_destroy(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */

	return 0;
}

int cache_init(struct cache *cache)
{
#if HAVE_PTHREADS
	if (pthread_mutex_init(&cache->lock, NULL) != 0)
		return 1;
	if (pthread_mutex_lock(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */

	/* FIXME: the maximum count is currently hardcoded */
	if ((cache->hash = hash_create(1 << 29 - 15, NULL, crc32)) == NULL) {
#if HAVE_PTHREADS
		pthread_mutex_unlock(&cache->lock);
		pthread_mutex_destroy(&cache->lock);
#endif /* HAVE_PTHREADS */
		return 1;
	}

#if HAVE_PTHREADS
	if (pthread_mutex_unlock(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */

	return 0;
}

int cache_insert(struct cache *cache, void *data, size_t len)
{
	hnode_t *hnode;

	if ((hnode = hnode_create(data)) == NULL)
		return 1;
#if HAVE_PTHREADS
	if (pthread_mutex_lock(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */
	hash_insert(cache->hash, hnode, data, len);
#if HAVE_PTHREADS
	if (pthread_mutex_unlock(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */

	return 0;
}

int cache_lookup(struct cache *cache, void *data, size_t len)
{
	hnode_t *hnode;

#if HAVE_PTHREADS
	if (pthread_mutex_lock(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */
	hnode = hash_lookup(cache->hash, data, len);
#if HAVE_PTHREADS
	if (pthread_mutex_lock(&cache->lock) != 0)
		return 1;
#endif /* HAVE_PTHREADS */
	if (hnode == NULL)
		return 1;

	return 0;
}
