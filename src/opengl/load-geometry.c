
#include <stddef.h>
#include <stdalign.h>
#include <GL/gl.h>

#include <rescache/rescache.h>
#include <wf/wf.h>

#include "types.h"
#include "load-wf.h"
#include "load-geometry.h"

static int load_wf_obj(void const *key, size_t len, void *data, void *link)
{
	struct glgeometries *geos = data;
	struct glstate *state = link;
	char const *filename = key;
	(void)len;

	return gl_load_wfobj(state, geos, filename);
}

static void free_geometry(void const *key, size_t len, void *data, void *link)
{
	struct glgeometries *geos = data;
	struct glstate *state = link;

	(void)key;
	(void)len;

	gl_free_wfgeo(state, geos);
}

struct rescache *gl_make_geometries_cache(struct glstate *state)
{
	/* key: filename string */
	return make_rescache(
		sizeof(struct glgeometries),
		alignof(struct glgeometries),
		alignof(char),
		load_wf_obj,
		free_geometry,
		state);
}

struct glgeometries const *gl_load_geometry(
	struct glcache *cache,
	char const *filename)
{
	return rescache_loads(cache->geometries, filename);
}

void gl_release_geometry(
	struct glcache *cache,
	struct glgeometries const *geometries)
{
	rescache_release(cache->geometries, geometries);
}

