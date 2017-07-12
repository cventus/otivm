#include <string.h>
#include <stdlib.h>
#include <stdalign.h>
#include <assert.h>

#include <wf/wf.h>
#include <rescache/rescache.h>

#include <glapi/core.h>
#include "include/types.h"
#include "include/cache.h"
#include "private.h"
#include "caches.h"
#include "decl.h"

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
	struct gl_cache *cache;
	struct wf_mtllib const *const *mtllib;
	struct wf_material const *wfmtl;
	struct gl_material *mtl;
	struct mtllib_key mkey;

	if (get_mtllib_key(key, size, &mkey)) { return -1; }

	cache = link;
	mtllib = gl_load_wf_mtllib(cache, mkey.filename);
	if (!mtllib) { return -1; }

	wfmtl = wf_get_material(*mtllib, mkey.name);
	if (wfmtl) {
		mtl = data;

		assert(sizeof mtl->ambient == sizeof wfmtl->ka);
		memcpy(mtl->ambient, wfmtl->ka, sizeof wfmtl->kd);

		assert(sizeof mtl->diffuse == sizeof wfmtl->kd);
		memcpy(mtl->diffuse, wfmtl->kd, sizeof wfmtl->kd);

		assert(sizeof mtl->specular == sizeof wfmtl->ks);
		memcpy(mtl->specular, wfmtl->ks, sizeof wfmtl->ks);

		mtl->exponent = wfmtl->ns;
		mtl->program = 0;

		result = 0;
	} else {
		result = -2;
	}
	gl_release_wf_mtllib(cache, mtllib);
	return result;
}

static void unload_material(void const *key, size_t len, void *data, void *link)
{
	(void)key;
	(void)len;
	(void)data;
	(void)link;
}

struct rescache *gl_make_materials_cache(struct gl_cache *cache)
{
	return make_rescache(
		sizeof(struct gl_material),
		alignof(struct gl_material),
		alignof(char),
		load_mtllib,
		unload_material,
		cache);
}

struct gl_material const *gl_load_wf_material(
	struct gl_cache *cache,
	char const *mtllib,
	char const *name)
{
	struct gl_material const *mat;
	void *key;
	size_t size;

	if (make_mtllib_key(mtllib, name, &key, &size)) { return NULL; }
	mat = rescache_load(cache->materials, key, size);
	free(key);
	return mat;
}

void gl_release_material(
	struct gl_cache *cache,
	struct gl_material const *material)
{
	rescache_release(cache->materials, material);
}
