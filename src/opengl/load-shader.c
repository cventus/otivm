
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <GL/gl.h>

#include <base/mem.h>
#include <fs/file.h>
#include <rescache/rescache.h>

#include "types.h"
#include "shader.h"

static int load_shader_file(
	char const *filename,
	size_t len,
	char const *ext,
	struct glshader *shader,
	struct glstate *state,
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
	result = gl_make_shader(state, shader, type, source);
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

static void free_shader(void const *key, size_t ksz, void *data, void *link)
{
	(void)key;
	(void)ksz;
	gl_free_shader(link, data);
}

struct rescache *gl_make_shaders_cache(struct glstate *state)
{
	loadfn *const load_shader[] = { load_vshader, load_fshader };

	/* key: filename string */
	return make_rescachen(
		sizeof(struct glshader),
		alignof(struct glshader),
		alignof(char),
		load_shader,
		length_of(load_shader),
		free_shader,
		state);
}

struct glshader const *gl_load_shader(
	struct glcache *cache,
	char const *filename)
{
	return rescache_loads(cache->shaders, filename);
}

void gl_release_shader(struct glcache *cache, struct glshader const *shader)
{
	rescache_release(cache->shaders, shader);
}

