
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>
#include <GL/gl.h>

#include <rescache/rescache.h>

#include "types.h"
#include "shader.h"
#include "load-shader.h"
#include "load-program.h"

struct program_key
{
	size_t n;
	char keys[];
};

static int string_cmp(const void *a, const void *b) { return strcmp(a, b); }

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

	*out = pk;
	*size = sz;

	return 0;
}

static int load_program(void const *key, size_t ksz, void *data, void *link)
{
	char const *p;
	struct glshader const **shaders;
	size_t i;
	int result;

	struct program_key const *pk = key;
	struct glprogram *program = data;
	struct glstate *state = link;

	(void)ksz;
	shaders = malloc(pk->n * sizeof *shaders);
	if (!shaders) { return -1; }

	for (i = 0, p = pk->keys; i < pk->n; i++, p += strlen(p) + 1) {
		shaders[i] = gl_load_shader(&state->cache, p);
		if (!shaders[i]) {
			while (i--) {
				gl_release_shader(&state->cache, shaders[i]);
			}
			free(shaders);
			return -2;
		}
	}
	result = gl_make_program(state, program, shaders, pk->n);
	for (i = 0; i < pk->n; i++) {
		gl_release_shader(&state->cache, shaders[i]);
	}
	free(shaders);
	return result;
}

static void free_program(void const *key, size_t ksz, void *data, void *link)
{
	(void)key;
	(void)ksz;
	gl_free_program(link, data);
}

struct rescache *gl_make_programs_cache(struct glstate *state)
{
	return make_rescache(
		sizeof(struct glprogram),
		alignof(struct glprogram),
		alignof(char),
		load_program,
		free_program,
		state);
}

struct glprogram const *gl_load_program(
	struct glcache *cache,
	char const *const *shader_keys,
	size_t nshader_keys)
{
	struct glprogram const *program;
	size_t size;
	void *key;

	if (make_key(shader_keys, nshader_keys, &key, &size)) { return NULL; }
	program = rescache_load(cache->programs, key, size);
	free(key);
	return program;
}

void gl_release_program(struct glcache *cache, struct glprogram const *program)
{
	rescache_release(cache->programs, program);
}

