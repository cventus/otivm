#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>

#include "base/mem.h"
#include "fs/file.h"
#include "rescache/rescache.h"
#include "glapi/api.h"
#include "glapi/core.h"
#include "glcache/types.h"
#include "glcache/cache.h"

#include "caches.h"
#include "private.h"
#include "shader.h"

static int load_shader_file(
	char const *filename,
	size_t len,
	char const *ext,
	struct gl_shader *shader,
	struct gl_cache *cache,
	GLenum type)
{
	FILE *fp;
	char *source;
	int result;
	size_t extlen;

	extlen = strlen(ext);
	if (len < extlen || strcmp(filename + (len - extlen), ext) != 0) {
		return -1;
	}
	fp = fopen(filename, "r");
	if (!fp) { return -1; }
	source = read_all(fp);
	if (!source) { fclose(fp); return -2; }
	result = gl_shader_init(cache->api, shader, type, source);
	free(source);
	fclose(fp);
	return result;
}

static int load_vshader(void const *key, size_t size, void *data, void *link)
{
	if (size == 0) { return -1; }
	return load_shader_file(
		key,
		size - 1,
		".vert",
		data,
		link,
		GL_VERTEX_SHADER);
}

static int load_fshader(void const *key, size_t size, void *data, void *link)
{
	if (size == 0) { return -1; }
	return load_shader_file(
		key,
		size - 1,
		".frag",
		data,
		link,
		GL_FRAGMENT_SHADER);
}

static void unload_shader(void const *key, size_t ksz, void *data, void *link)
{
	(void)key;
	(void)ksz;
	struct gl_cache *cache = link;
	gl_shader_term(cache->api, (struct gl_shader *)data);
}

struct rescache *gl_make_shaders_cache(struct gl_cache *cache)
{
	loadfn *const loaders[] = { load_vshader, load_fshader };

	/* key: filename string */
	return make_rescachen(
		sizeof(struct gl_shader),
		alignof(struct gl_shader),
		alignof(char),
		loaders,
		length_of(loaders),
		unload_shader,
		cache);
}

struct gl_shader const *gl_load_shader(
	struct gl_cache *cache,
	char const *filename)
{
	return rescache_loads(cache->shaders, filename);
}

void gl_release_shader(struct gl_cache *cache, struct gl_shader const *shader)
{
	rescache_release(cache->shaders, shader);
}
