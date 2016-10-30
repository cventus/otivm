
#include <stddef.h>
#include <stdalign.h>
#include <GL/gl.h>

#include <base/mem.h>
#include <rescache/rescache.h>

#include "include/types.h"
#include "include/cache.h"
#include "caches.h"
#include "private.h"

static int load_png(void const *filename, size_t len, void *data, void *link)
{
	(void)filename;
	(void)len;
	(void)data;
	(void)link;
	return -1;
}

static int load_tga(void const *filename, size_t len, void *data, void *link)
{
	(void)filename;
	(void)len;
	(void)data;
	(void)link;
	return -1;
}

static void unload_texture(
	void const *filename,
	size_t len,
	void *data,
	void *link)
{
	(void)filename;
	(void)len;
	(void)data;
	(void)link;
}

struct rescache *gl_make_textures_cache(struct gl_cache *cache)
{
	loadfn *const loaders[] = { load_png, load_tga };

	/* key: filename string */
	return make_rescachen(
		sizeof(struct gl_texture),
		alignof(struct gl_texture),
		alignof(char),
		loaders,
		length_of(loaders),
		unload_texture,
		cache);
}

struct gl_texture const *gl_load_texture(
	struct gl_cache *cache,
	char const *filename)
{
	return rescache_loads(cache->textures, filename);
}

void gl_release_texture(
	struct gl_cache *cache,
	struct gl_texture const *texture)
{
	rescache_release(cache->textures, texture);
}

