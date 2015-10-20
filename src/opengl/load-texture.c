
#include <stddef.h>
#include <stdalign.h>
#include <GL/gl.h>

#include <base/mem.h>
#include <rescache/rescache.h>

#include "types.h"
#include "load-texture.h"

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

static void free_texture(
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

struct rescache *gl_make_textures_cache(struct glstate *state)
{
	loadfn *const load_texture[] = { load_png, load_tga };

	/* key: filename string */
	return make_rescachen(
		sizeof(struct gltexture),
		alignof(struct gltexture),
		alignof(char),
		load_texture,
		length_of(load_texture),
		free_texture,
		state);
}

struct gltexture const *gl_load_texture(
	struct glcache *cache,
	char const *filename)
{
	return rescache_loads(cache->textures, filename);
}

void gl_release_texture(struct glcache *cache, struct gltexture const *texture)
{
	rescache_release(cache->textures, texture);
}

