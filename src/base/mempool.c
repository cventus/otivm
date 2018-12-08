#include <stddef.h>
#include <stdalign.h>
#include <stdlib.h>
#include <assert.h>

#include "base/wbuf.h"
#include "base/fixpool.h"
#include "base/mempool.h"

void mempool_init(struct mempool *pool, size_t buffer_nmemb, size_t size)
{
	assert(size > 0);
	assert(buffer_nmemb > 0);
	assert(fixpool_aligned(size));
	pool->nmemb = 0;
	pool->buffer_nmemb = buffer_nmemb;
	pool->size = size;
	pool->free = NULL;
	wbuf_init(&pool->buffers);
}

void mempool_term(struct mempool *pool)
{
	assert(pool != NULL);
	void **p = pool->buffers.begin;
	void **q = pool->buffers.end;
	for (; p < q; p++) { free(*p); }
	wbuf_term(&pool->buffers);
	pool->nmemb = 0;
	pool->buffer_nmemb = 0;
	pool->size = 0;
	pool->free = NULL;
}

size_t mempool_capacity(struct mempool *pool)
{
	assert(pool != NULL);
	size_t bufsiz = pool->buffer_nmemb * pool->size;
	return bufsiz * wbuf_nmemb(&pool->buffers, sizeof (void *));
}

void *mempool_alloc(struct mempool *pool)
{
	assert(pool != NULL);
	if (!pool->free) {
		if (wbuf_reserve(&pool->buffers, sizeof (void *))) {
			return NULL;
		}
		struct fixpool fp;
		size_t bufsiz = pool->buffer_nmemb * pool->size;
		void *p = malloc(bufsiz);
		if (!p) { return NULL; }
		fixpool_init(&fp, p, pool->buffer_nmemb, pool->size);
		(void)wbuf_write(&pool->buffers, &fp.buffer, sizeof fp.buffer);
		pool->free = fp.free;
	}
	void *p = pool->free;
	pool->free = *pool->free;
	pool->nmemb++;
	return p;
}

void mempool_free(struct mempool *pool, void *p)
{
	assert(pool != NULL);
	if (p) {
		void **q = p;
		*q = pool->free;
		pool->free = q;
		pool->nmemb--;
	}
}
