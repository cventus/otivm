#include <stddef.h>
#include <stdalign.h>
#include <assert.h>
#include "base/fixpool.h"

void fixpool_init(struct fixpool *pool, void *buffer, size_t nmemb, size_t size)
{
	size_t i;
	void **p;

	assert(fixpool_aligned(size));

	pool->buffer = buffer;
	for (p = buffer, i = 1; i < nmemb; i++) {
		*p = (char *)p + size;
		p = *p;
	}
	if (nmemb > 0) {
		pool->free = buffer;
		*p = NULL;
	} else {
		pool->free = NULL;
	}
}

int fixpool_is_empty(struct fixpool *pool)
{
	return pool->free == NULL;
}

void *fixpool_alloc(struct fixpool *pool)
{
	if (!pool->free) { return NULL; }
	void *p = pool->free;
	pool->free = *pool->free;
	return p;
}

void fixpool_free(struct fixpool *pool, void *p)
{
	if (p) {
		void **q = p;
		*q = pool->free;
		pool->free = q;
	}
}
