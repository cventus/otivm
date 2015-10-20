
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include <base/mem.h>
#include <text/re.h>
#include <text/str.h>
#include <rescache/rescache.h>
#include <wf/wf.h>

#include "types.h"
#include "decl.h"
#include "shader.h"

#include "load-material.h"
#include "load-mtllib.h"
#include "load-geometry.h"
#include "load-texture.h"
#include "load-program.h"
#include "load-shader.h"

#define cache_field(name) \
	{ offsetof(struct glcache, name), #name, gl_make_##name##_cache }
#define get_cache_field(c,f) (struct rescache **)((char *)&(c) + (f).offset)

typedef struct rescache *makecache(struct glstate *);

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

int gl_init_cache(struct glcache *cache, struct glstate *state)
{
	size_t i;
	struct rescache **field;

	assert(&state->cache == cache);

	for (i = 0; i < length_of(cache_fields); i++) {
		field = get_cache_field(*cache, cache_fields[i]);
		*field = cache_fields[i].constructor(state);
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

size_t gl_clean_caches(struct glcache *cache)
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

int gl_free_cache(struct glcache *cache)
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
