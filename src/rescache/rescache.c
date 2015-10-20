
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>
#include <assert.h>

#include <base/mem.h>

#include "include/rescache.h"

typedef int ctor(void const *key, size_t key_size, void *data, void *link);
typedef void dtor(void const *key, size_t key_size, void *data, void *link);

struct resource
{
	struct resource *next;
	unsigned int refc;
	size_t key_size;
};

struct rescache
{
	struct resource *reslist;
	size_t const nloaders, data_size, data_offset, key_offset;
	void *const link;
	dtor *const unload;
	ctor *const loaders[];
};

struct rescache *make_rescache(
	size_t data_size,
	size_t data_align,
	size_t key_align,
	ctor *const load,
	dtor *unload,
	void *link)
{
	return make_rescachen(
		data_size,
		data_align,
		key_align,
		&load,
		1,
		unload,
		link);
}

struct rescache *make_rescachen(
	size_t data_size,
	size_t data_align,
	size_t key_align,
	ctor *const loaders[],
	size_t nloaders,
	dtor *unload,
	void *link)
{
	struct rescache *cache;

	assert(data_size > 0);
	assert(loaders || nloaders == 0);
	assert(unload);

	if (!(cache = malloc(sizeof *cache + nloaders*sizeof *loaders))) {
		return NULL;
	}
	cache->reslist = NULL;
	*(dtor **)&cache->unload = unload;
	*(size_t *)&cache->nloaders = nloaders;
	*(size_t *)&cache->data_size = data_size;
	*(size_t *)&cache->data_offset = align_to(
		sizeof (struct resource),
		data_align);
	*(size_t *)&cache->key_offset = align_to(
		cache->data_offset + cache->data_size,
		key_align);
	*(void **)&cache->link = link;
	memcpy((ctor **)cache->loaders, loaders, nloaders * sizeof *loaders);
	return cache;
}

static void *res_key(struct rescache *cache, struct resource *res)
{
	assert(cache);
	assert(res);
	return (char *)res + cache->key_offset;
}

static void *res_data(struct rescache *cache, struct resource *res)
{
	assert(cache);
	assert(res);
	return (char *)res + cache->data_offset;
}

static struct resource *add_res(
	struct rescache *cache,
	void const *key,
	size_t key_size)
{
	struct resource *res;
	size_t i;
	char const *rkey;
	void *data;

	assert(cache);
	assert(key);

	res = malloc(cache->key_offset + key_size);
	if (!res) { return NULL; }
	rkey = memcpy(res_key(cache, res), key, key_size);
	data = res_data(cache, res);
	for (i = 0; i < cache->nloaders; i++) {
		if (cache->loaders[i](rkey, key_size, data, cache->link) == 0) {
			res->refc = 1;
			res->next = cache->reslist;
			res->key_size = key_size;
			cache->reslist = res;
			return res;
		}
	}
	free(res);
	return NULL;
}

static struct resource **find_res(
	struct rescache *cache,
	char const *key,
	size_t const ksz)
{
	struct resource **res;

	assert(cache);
	assert(key);

	for (res = &cache->reslist; *res; res = &(*res)->next) {
		size_t const sz = (*res)->key_size;
		if (ksz == sz && !memcmp(key, res_key(cache, *res), ksz)) {
			break;
		}
	}
	return res;
}

static struct resource **find_data(struct rescache *cache, void const *data)
{
	struct resource **res;

	assert(cache);
	assert(data);

	for (res = &cache->reslist; *res; res = &(*res)->next) {
		if (data == res_data(cache, *res)) { break; }
	}
	return res;
}

int free_rescache(struct rescache *cache)
{
	assert(cache);

	rescache_clean(cache);
	if (cache->reslist) { return -1; }
	free(cache);
	return 0;
}

size_t rescache_size(struct rescache *cache)
{
	struct resource *res;
	size_t sz;

	assert(cache);

	for (sz = 0, res = cache->reslist; res; res = res->next, sz++) { }
	return sz;
}

size_t rescache_unused(struct rescache *cache)
{
	struct resource *res;
	size_t sz;

	assert(cache);

	for (sz = 0, res = cache->reslist; res; res = res->next) {
		if (res->refc == 0) { sz++; }
	}
	return sz;
}

static void free_res(struct rescache *cache, struct resource **res)
{
	struct resource *s;

	assert(cache);
	assert(res);

	s = *res;
	*res = (*res)->next;
	cache->unload(
		res_key(cache, s),
		s->key_size,
		res_data(cache, s),
		cache->link);
	free(s);
}

size_t rescache_clean(struct rescache *cache)
{
	size_t n;
	struct resource **res;

	assert(cache);

	for (n = 0, res = &cache->reslist; *res; ) {
		if ((*res)->refc == 0) {
			free_res(cache, res);
			n++;
		} else {
			res = &(*res)->next;
		}
	}
	return n;
}

void *rescache_load(struct rescache *cache, void const *key, size_t key_size)
{
	struct resource *res;

	assert(cache);
	if (!key && key_size > 0) { return NULL; }

	res = *find_res(cache, key, key_size);
	if (res) {
		res->refc++;
	} else {
		res = add_res(cache, key, key_size);
		if (!res) { return NULL; }
	}
	return res_data(cache, res);
}

void *rescache_loads(struct rescache *cache, char const *key)
{
	assert(cache);
	if (!key) { return NULL; }
	return rescache_load(cache, key, strlen(key) + 1);
}

static struct resource **release(struct rescache *cache, void const *data)
{
	struct resource **res, *s;

	assert(cache);
	if (!data) { return NULL; }
	res = find_data(cache, data);
	if (!(s = *res)) { return NULL; }
	assert(s->refc > 0);
	s->refc--;

	return res;
}

void rescache_release(struct rescache *cache, void const *data)
{
	assert(cache);
	(void)release(cache, data);
}

void rescache_unload(struct rescache *cache, void const *data)
{
	struct resource **res;

	assert(cache);
	res = release(cache, data);
	if (res && (*res)->refc == 0) {
		free_res(cache, res);
	}
}

