
#include <string.h>
#include <stdlib.h>
#include <stdalign.h>
#include <GL/gl.h>

#include <wf/wf.h>
#include <rescache/rescache.h>

#include "types.h"
#include "load-mtllib.h"

static int load_wf_mtllib(
	void const *filename,
	size_t len,
	void *data,
	void *link)
{
	struct wf_mtllib const **p;

	(void)link;
	(void)len;
	p = data;
	*p = wf_parse_mtllib(filename);
	return *p ? 0 : -1;
}

static void free_wf_mtllib(void const *key, size_t len, void *data, void *link)
{
	(void)key;
	(void)len;
	(void)link;
	wf_free_mtllib(*(struct wf_mtllib const **)data);
}

struct rescache *gl_make_wf_mtllibs_cache(struct glstate *state)
{
	/* key: filename string */
	return make_rescache(
		sizeof(struct wf_mtllib *),
		alignof(struct wf_mtllib *),
		alignof(char),
		load_wf_mtllib,
		free_wf_mtllib,
		state);
}

struct wf_mtllib const *const *gl_load_wf_mtllib(
	struct glcache *cache,
	char const *key)
{
	return rescache_loads(cache->wf_mtllibs, key);
}

void gl_release_wf_mtllib(
	struct glcache *cache,
	struct wf_mtllib const *const *mtllib)
{
	rescache_release(cache->wf_mtllibs, mtllib);
}

