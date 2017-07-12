#include <stddef.h>
#include <stdalign.h>

#include <rescache/rescache.h>
#include <glapi/core.h>
#include <wf/wf.h>

#include "include/types.h"
#include "include/cache.h"
#include "private.h"
#include "geometry.h"
#include "caches.h"

static int load_wf_obj(void const *key, size_t len, void *data, void *link)
{
	struct gl_geometries *geos = data;
	struct gl_cache *cache = link;
	char const *filename = key;
	(void)len;

	return gl_geometries_init_wfobj(cache, geos, filename);
}

static void unload_geometry(void const *key, size_t len, void *data, void *link)
{
	struct gl_geometries *geos = data;
	struct gl_cache *cache = link;

	(void)key;
	(void)len;

	gl_geometries_term(cache, geos);
}

struct rescache *gl_make_geometries_cache(struct gl_cache *cache)
{
	/* key: filename string */
	return make_rescache(
		sizeof(struct gl_geometries),
		alignof(struct gl_geometries),
		alignof(char),
		load_wf_obj,
		unload_geometry,
		cache);
}

struct gl_geometries const *gl_load_geometry(
	struct gl_cache *cache,
	char const *filename)
{
	return rescache_loads(cache->geometries, filename);
}

void gl_release_geometry(
	struct gl_cache *cache,
	struct gl_geometries const *geometries)
{
	rescache_release(cache->geometries, geometries);
}
