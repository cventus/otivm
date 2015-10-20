
#include <stddef.h>
#include <stdalign.h>
#include <GL/gl.h>

#include <rescache/rescache.h>
#include <wf/wf.h>

#include "types.h"
#include "load-wf.h"

static int load_wf_obj(void const *filename, size_t len, void *data, void *link)
{
	struct glgeometries const **p = data;
	struct glstate *state = link;

	(void)len;

	*p = gl_load_wfobj(state, filename);
	return *p ? 0 : -1;
}

static void free_geometry(void const *key, size_t len, void *data, void *link)
{
	struct glgeometries const **geos = data;
	struct glstate *state = link;

	(void)key;
	(void)len;

	gl_free_wfgeo(state, *geos);
}

struct rescache *gl_make_geometries_cache(struct glstate *state)
{
	/* key: filename string */
	return make_rescache(
		sizeof(struct glgeometries *),
		alignof(struct glgeometries *),
		alignof(char),
		load_wf_obj,
		free_geometry,
		state);
}

struct glgeometries const *const *gl_load_geometry(
	struct glcache *cache,
	char const *filename)
{
	return rescache_loads(cache->geometries, filename);
}

void gl_release_geometry(
	struct glcache *cache,
	struct glgeometries const *const *geometry)
{
	rescache_release(cache->geometries, geometry);
}

