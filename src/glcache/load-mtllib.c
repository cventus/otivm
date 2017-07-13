#include <string.h>
#include <stdlib.h>
#include <stdalign.h>

#include <wf/wf.h>
#include <rescache/rescache.h>
#include <glapi/core.h>

#include "include/types.h"
#include "private.h"
#include "caches.h"
#include "decl.h"

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

static void unload_wf_mtllib(
	void const *key,
	size_t len,
	void *data,
	void *link)
{
	(void)key;
	(void)len;
	(void)link;
	wf_free_mtllib(*(struct wf_mtllib const **)data);
}

struct rescache *gl_make_wf_mtllibs_cache(struct gl_cache *cache)
{
	/* key: filename string */
	return make_rescache(
		sizeof(struct wf_mtllib *),
		alignof(struct wf_mtllib *),
		alignof(char),
		load_wf_mtllib,
		unload_wf_mtllib,
		cache);
}

struct wf_mtllib const *const *gl_load_wf_mtllib(
	struct gl_cache *cache,
	char const *key)
{
	return rescache_loads(cache->wf_mtllibs, key);
}

void gl_release_wf_mtllib(
	struct gl_cache *cache,
	struct wf_mtllib const *const *mtllib)
{
	rescache_release(cache->wf_mtllibs, mtllib);
}
