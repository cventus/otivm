#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <base/mem.h>
#include <text/re.h>
#include <text/str.h>
#include <rescache/rescache.h>
#include <wf/wf.h>
#include <glapi/core.h>

#include "include/types.h"
#include "private.h"
#include "decl.h"
#include "shader.h"

#include "caches.h"

#define cache_field(name) \
	{ offsetof(struct gl_cache, name), #name, gl_make_##name##_cache }
#define get_cache_field(c,f) (struct rescache **)((char *)&(c) + (f).offset)

typedef struct rescache *makecache(struct gl_cache *);

struct field
{
	size_t offset;
	char const *name;
	makecache *constructor;
};

static struct field const cache_fields[] = {
	cache_field(textures),
	cache_field(shaders),
	cache_field(programs),
	cache_field(materials),
	cache_field(geometries),
	cache_field(wf_mtllibs)
};

int gl_cache_init(struct gl_cache *cache, struct gl_api *api)
{
	size_t i;
	struct rescache **field;

	cache->api = api;
	for (i = 0; i < length_of(cache_fields); i++) {
		field = get_cache_field(*cache, cache_fields[i]);
		*field = cache_fields[i].constructor(cache);
		if (!*field) {
			while (i-- > 0) {
				field = get_cache_field(
					*cache,
					cache_fields[i]);
				free_rescache(*field);
			}
			return -1;
		}
	}

	return 0;
}


struct gl_cache *gl_make_cache(struct gl_api *api)
{
	struct gl_cache *cache;
	cache = malloc(sizeof *cache);
	if (gl_cache_init(cache, api)) {
		free(cache);
		cache = NULL;
	}
	return cache;
}

int gl_free_cache(struct gl_cache *cache)
{
	int n;
	n = gl_cache_term(cache);
	if (!n) {
		free(cache);
	}
	return n;
}

size_t gl_clean_caches(struct gl_cache *cache)
{
	size_t i, nfreed, ntotal;
	struct rescache **field;

	ntotal = 0;
	do {
		/* Free resources as much as possible */
		nfreed = 0;
		for (i = 0; i < length_of(cache_fields); i++) {
			field = get_cache_field(*cache, cache_fields[i]);
			if (*field != NULL) {
				nfreed += rescache_clean(*field);
			}
		}
		ntotal += nfreed;
	} while (nfreed > 0);

	return ntotal;
}

int gl_cache_term(struct gl_cache *cache)
{
	size_t i;
	struct rescache **field;
	int nfailed;

	gl_clean_caches(cache);

	nfailed = 0;
	for (i = 0; i < length_of(cache_fields); i++) {
		field = get_cache_field(*cache, cache_fields[i]);
		if (*field != NULL) {
			if (free_rescache(*field)) { nfailed++; }
			*field = NULL;
		}
	}
	return nfailed;
}
