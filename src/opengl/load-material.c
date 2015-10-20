
#include <string.h>
#include <stdlib.h>
#include <stdalign.h>
#include <GL/gl.h>

#include <wf/wf.h>
#include <rescache/rescache.h>

#include "types.h"
#include "load-mtllib.h"
#include "load-material.h"

#define MTLLIB_PREFIX "mtllib"

struct mtllib_key
{
	char const *filename;
	char const *name;
};

static int make_mtllib_key(
	char const *mtllib,
	char const *name,
	void **key,
	size_t *size)
{
	char *p, *q;
	size_t fsz, nsz, qsz, sz;

	q = MTLLIB_PREFIX;
	fsz = strlen(mtllib) + 1;
	nsz = strlen(name) + 1;
	qsz = strlen(q) + 1;

	sz = fsz + nsz + qsz;
	p = malloc(sz);
	if (!sz) { return -1; }
	(void)memcpy(p, q, qsz);
	(void)memcpy(p + qsz, name, nsz);
	(void)memcpy(p + qsz + nsz, mtllib, fsz);

	*key = p;
	*size = sz;

	return 0;
}

static int get_mtllib_key(void const *key, size_t size, struct mtllib_key *out)
{
	if (!key || size == 0 || ((char *)key)[size - 1] != '\0') { return -1; }
	if (strcmp(key, MTLLIB_PREFIX) != 0) { return -1; }

	out->name = (char *)key + strlen(MTLLIB_PREFIX) + 1;
	out->filename = out->name + strlen(out->name) + 1;
	return 0;
}

static int load_mtllib(void const *key, size_t size, void *data, void *link)
{
	int result;
	struct glstate *state;
	struct wf_mtllib const *const *mtllib;
	struct wf_material const *wfmtl;
	struct glmaterial *mtl;
	struct mtllib_key mkey;

	if (get_mtllib_key(key, size, &mkey)) { return -1; }

	state = link;
	mtllib = gl_load_wf_mtllib(&state->cache, mkey.filename);
	if (!mtllib) { return -1; }

	wfmtl = wf_get_material(*mtllib, mkey.name);
	if (wfmtl) {
		mtl = data;
		mtl->diffuse[0] = wfmtl->kd[0];
		result = 0;
	} else {
		result = -2;
	}
	gl_release_wf_mtllib(&state->cache, mtllib);
	return result;
}

static void free_material(void const *key, size_t len, void *data, void *link)
{
	(void)key;
	(void)len;
	(void)data;
	(void)link;
}

struct rescache *gl_make_materials_cache(struct glstate *state)
{
	return make_rescache(
		sizeof(struct glmaterial),
		alignof(struct glmaterial),
		alignof(char),
		load_mtllib,
		free_material,
		state);
}

struct glmaterial const *gl_load_wf_material(
	struct glcache *cache,
	char const *mtllib,
	char const *name)
{
	struct glmaterial const *mat;
	void *key;
	size_t size;

	if (make_mtllib_key(mtllib, name, &key, &size)) { return NULL; }
	mat = rescache_load(cache->materials, key, size);
	free(key);
	return mat;
}

void gl_release_material(
	struct glcache *cache,
	struct glmaterial const *material)
{
	rescache_release(cache->materials, material);
}

