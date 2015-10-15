
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>
#include <assert.h>

#include <base/mem.h>

#include "include/rescache.h"

typedef int ctor(char const *key, void *data, void *link);
typedef void dtor(char const *key, void *data, void *link);

struct resource
{
	struct resource *next;
	unsigned int refc;
};

struct rescache
{
	struct resource *reslist;
	size_t const nmake, data_size, data_offset;
	void *const link;
	dtor *const free;
	ctor *const make[];
};

struct rescache *make_rescache(size_t data_size, size_t data_align,
			       ctor *const make, dtor *free, void *link)
{
	return make_rescachen(data_size, data_align, &make, 1, free, link);
}

struct rescache *make_rescachen(size_t data_size, size_t data_align,
			        ctor *const make[], size_t nmake, dtor *free,
                                void *link)
{
	struct rescache *cache;

	assert(data_size > 0);
	assert(make || nmake == 0);
	assert(free);

	if (!(cache = malloc(sizeof *cache + nmake*sizeof *make))) {
		return NULL;
	}
	cache->reslist = NULL;
	*(dtor **)&cache->free = free;
	*(size_t *)&cache->nmake = nmake;
	*(size_t *)&cache->data_size = data_size;
	*(size_t *)&cache->data_offset = align_to(
		sizeof (struct resource),
		data_align);
	*(void **)&cache->link = link;
	memcpy((ctor **)cache->make, make, nmake * sizeof *make);
	return cache;
}

static char *res_key(struct rescache *cache, struct resource *res)
{
	assert(cache);
	assert(res);
	return (char *)res + cache->data_offset + cache->data_size;
}

static void *res_data(struct rescache *cache, struct resource *res)
{
	assert(cache);
	assert(res);
	return (char *)res + cache->data_offset;
}

static struct resource *add_res(struct rescache *cache, char const *key)
{
	struct resource *res;
	size_t i;
	char const *rkey;
	void *data;

	assert(cache);
	assert(key);

	if (!key) { return NULL; }
	res = malloc(cache->data_offset + cache->data_size + strlen(key) + 1);
	if (!res) { return NULL; }
	rkey = strcpy(res_key(cache, res), key);
	data = res_data(cache, res);
	for (i = 0; i < cache->nmake; i++) {
		if (cache->make[i](rkey, data, cache->link) == 0) {
			res->refc = 1;
			res->next = cache->reslist;
			cache->reslist = res;
			return res;
		}
	}
	free(res);
	return NULL;
}

static struct resource **find_res(struct rescache *cache, char const *key)
{
	struct resource **res;

	assert(cache);
	assert(key);

	for (res = &cache->reslist; *res; res = &(*res)->next) {
		if (strcmp(key, res_key(cache, *res)) == 0) { break; }
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
	cache->free(res_key(cache, s), res_data(cache, s), cache->link);
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

void *rescache_load(struct rescache *cache, char const *key)
{
	struct resource *res;

	assert(cache);
	if (!key) { return NULL; }

	res = *find_res(cache, key);
	if (res) {
		res->refc++;
	} else {
		res = add_res(cache, key);
		if (!res) { return NULL; }
	}
	return res_data(cache, res);
}

void rescache_release(struct rescache *cache, void const *data)
{
	struct resource **res, *s;

	assert(cache);
	if (!data) { return; }
	res = find_data(cache, data);
	if (!(s = *res)) { return; }
	assert(s->refc > 0);
	s->refc--;
}

