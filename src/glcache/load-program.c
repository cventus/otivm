
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>
#include <GL/gl.h>

#include <rescache/rescache.h>

#include "include/types.h"
#include <opengl/core.h>
#include "include/cache.h"
#include "caches.h"
#include "private.h"
#include "shader.h"

struct program_key
{
	size_t n;
	char keys[];
};

static int string_cmp(const void *a, const void *b) {
	char const *a_str = *(char const *const *)a;
	char const *b_str = *(char const *const *)b;
	return strcmp(a_str, b_str);
}

static int make_key(char const *const *keys, size_t n, void **out, size_t *size)
{
	struct program_key *pk;
	size_t i, j, len, sz;
	char const **sorted_keys;

	sorted_keys = malloc(n * sizeof *sorted_keys);
	if (!sorted_keys) { return -1; }
	(void)memcpy(sorted_keys, keys, n * sizeof *keys);

	sz = sizeof *pk;
	for (i = 0; i < n; i++) {
		/* FIXME: Potential overflow */
		sz += strlen(keys[i]) + 1;
	}

	pk = malloc(sz);
	if (!pk) { free(sorted_keys); return -2; }

	qsort(sorted_keys, n, sizeof *sorted_keys, string_cmp);

	(void)memset(pk, 0, sz);
	for (i = 0, j = 0; i < n; i++) {
		len = strlen(sorted_keys[i]);
		(void)memcpy(pk->keys + j, sorted_keys[i], len);
		j += len + 1;
	}
	free(sorted_keys);

	pk->n = n;
	*out = pk;
	*size = sz;

	return 0;
}

static int load_program(void const *key, size_t ksz, void *data, void *link)
{
	char const *p;
	struct gl_shader const **shaders;
	size_t i;
	int result;

	struct program_key const *pk = key;
	struct gl_program *program = data;
	struct gl_cache *cache = link;

	(void)ksz;
	shaders = malloc(pk->n * sizeof *shaders);
	if (!shaders) { return -1; }

	for (i = 0, p = pk->keys; i < pk->n; i++, p += strlen(p) + 1) {
		shaders[i] = gl_load_shader(cache, p);
		if (!shaders[i]) {
			while (i--) {
				gl_release_shader(cache, shaders[i]);
			}
			free(shaders);
			return -2;
		}
	}
	result = gl_program_init(cache->state, program, shaders, pk->n);
	for (i = 0; i < pk->n; i++) {
		gl_release_shader(cache, shaders[i]);
	}
	free(shaders);
	return result;
}

static void unload_program(void const *key, size_t ksz, void *data, void *link)
{
	struct gl_cache *cache = link;
	(void)key;
	(void)ksz;
	gl_program_term(cache->state, data);
}

struct rescache *gl_make_programs_cache(struct gl_cache *cache)
{
	return make_rescache(
		sizeof(struct gl_program),
		alignof(struct gl_program),
		alignof(char),
		load_program,
		unload_program,
		cache);
}

struct gl_program const *gl_load_program(
	struct gl_cache *cache,
	char const *const *shader_keys,
	size_t nshader_keys)
{
	struct gl_program const *program;
	size_t size;
	void *key;

	if (make_key(shader_keys, nshader_keys, &key, &size)) { return NULL; }
	program = rescache_load(cache->programs, key, size);
	free(key);
	return program;
}

void gl_release_program(
	struct gl_cache *cache,
	struct gl_program const *program)
{
	rescache_release(cache->programs, program);
}

