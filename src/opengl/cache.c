
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include <base/mem.h>
#include <text/re.h>
#include <text/str.h>
#include <rescache/rescache.h>
#include <wf/wf.h>

#include "types.h"
#include "decl.h"

#define cache_field(name) \
	{ offsetof(struct glcache, name), #name, make_##name##_cache }
#define get_cache_field(c,f) (struct rescache **)((char *)&(c) + (f).offset)

typedef int loadfn(void const *, size_t, void *, void *);
typedef void freefn(void const *, size_t, void *, void *);
typedef struct rescache *makecache(struct glstate *);

struct field
{
	size_t offset;
	char const *name;
	makecache *constructor;
};

static loadfn
	load_png,
	load_tga,
	load_vshader,
	load_fshader,
	load_wf_obj,
	load_wf_mtllib,
	load_wf_material;

static freefn
	free_texture,
	free_shader,
	free_program,
	free_material,
	free_geometry,
	free_wf_mtllib;

static makecache
	make_textures_cache,
	make_vshaders_cache,
	make_fshaders_cache,
	make_programs_cache,
	make_materials_cache,
	make_geometries_cache,
	make_wf_mtllibs_cache;

static struct field const cache_fields[] = {
	cache_field(textures),
	cache_field(vshaders),
	cache_field(fshaders),
	cache_field(programs),
	cache_field(materials),
	cache_field(geometries),
	cache_field(wf_mtllibs)
};

static struct rescache *make_textures_cache(struct glstate *state)
{
	loadfn *load_texture[] = { load_png, load_tga };

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

static struct rescache *make_vshaders_cache(struct glstate *state)
{
	/* key: filename string */
	return make_rescache(
		sizeof(struct glshader),
		alignof(struct glshader),
		alignof(char),
		load_vshader,
		free_shader,
		state);
}

static struct rescache *make_fshaders_cache(struct glstate *state)
{
	/* key: filename */
	return make_rescache(
		sizeof(struct glshader),
		alignof(struct glshader),
		alignof(char),
		load_fshader,
		free_shader,
		state);
}

static struct rescache *make_programs_cache(struct glstate *state)
{
	/* key: programkey */
	return make_rescachen(
		sizeof(struct glprogram),
		alignof(struct glprogram),
		alignof(char),
		NULL,
		0,
		free_program,
		state);
}

static struct rescache *make_materials_cache(struct glstate *state)
{
	return make_rescache(
		sizeof(struct glmaterial),
		alignof(struct glmaterial),
		alignof(char),
		load_wf_material,
		free_material,
		state);
}

static struct rescache *make_geometries_cache(struct glstate *state)
{
	/* key: filename string */
	return make_rescache(
		sizeof(struct glgeometries *),
		alignof(struct glgeometries *),
		alignof(char),
		load_wf_obj,
		free_geometry,
		state);
}

static struct rescache *make_wf_mtllibs_cache(struct glstate *state)
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

int gl_init_cache(struct glcache *cache, struct glstate *state)
{
	size_t i;
	struct rescache **field;

	assert(&state->cache == cache);

	for (i = 0; i < length_of(cache_fields); i++) {
		field = get_cache_field(*cache, cache_fields[i]);
		*field = cache_fields[i].constructor(state);
		if (!*field) {
			while (i-- > 0) {
				field = get_cache_field(
					*cache,
					cache_fields[i]);
				free_rescache(*field);
			}
			return -1;
		}
	}

	return 0;
}

size_t gl_clean_caches(struct glcache *cache)
{
	size_t i, nfreed, ntotal;
	struct rescache **field;

	ntotal = 0;
	do {
		/* Free resources as much as possible */
		nfreed = 0;
		for (i = 0; i < length_of(cache_fields); i++) {
			field = get_cache_field(*cache, cache_fields[i]);
			if (*field != NULL) {
				nfreed += rescache_clean(*field);
			}
		}
		ntotal += nfreed;
	} while (nfreed > 0);

	return ntotal;
}

int gl_free_cache(struct glcache *cache)
{
	size_t i;
	struct rescache **field;
	int nfailed;

	gl_clean_caches(cache);

	nfailed = 0;
	for (i = 0; i < length_of(cache_fields); i++) {
		field = get_cache_field(*cache, cache_fields[i]);
		if (*field != NULL) {
			if (free_rescache(*field)) { nfailed++; }
			*field = NULL;
		}
	}
	return nfailed;
}

struct gltexture *gl_load_texture(struct glcache *cache, char const *filename)
{
	return rescache_loads(cache->textures, filename);
}

void gl_release_texture(struct glcache *cache, struct gltexture *texture)
{
	rescache_release(cache->textures, texture);
}

struct glshader *gl_load_vertex_shader(
	struct glcache *cache,
	char const *filename)
{
	return rescache_loads(cache->vshaders, filename);
}

void gl_release_vertex_shader(struct glcache *cache, struct glshader *shader)
{
	rescache_release(cache->vshaders, shader);
}

struct glshader *gl_load_fragment_shader(
	struct glcache *cache,
	char const *filename)
{
	return rescache_loads(cache->fshaders, filename);
}

void gl_release_fragment_shader(struct glcache *cache, struct glshader *shader)
{
	rescache_release(cache->fshaders, shader);
}

struct glmaterial const *gl_load_material(
	struct glcache *cache,
	char const *key)
{
	return rescache_loads(cache->materials, key);
}

void gl_release_material(
	struct glcache *cache,
	struct glmaterial const *material)
{
	rescache_release(cache->materials, material);
}

struct glgeometries const *const *gl_load_geometry(
	struct glcache *cache,
	char const *filename)
{
	return rescache_loads(cache->geometries, filename);
}

void gl_release_geometry(
	struct glcache *cache,
	struct glgeometries const *const *geometry)
{
	rescache_release(cache->geometries, geometry);
}

struct wf_mtllib const **gl_load_wf_mtllib(
	struct glcache *cache,
	char const *key)
{
	return rescache_loads(cache->wf_mtllibs, key);
}

void gl_release_wf_mtllib(
	struct glcache *cache,
	struct wf_mtllib const **mtllib)
{
	rescache_release(cache->wf_mtllibs, mtllib);
}

static int load_png(void const *filename, size_t len, void *data, void *link)
{
	(void)filename;
	(void)len;
	(void)data;
	(void)link;
	return 0;
}

static int load_tga(void const *filename, size_t len, void *data, void *link)
{
	(void)filename;
	(void)len;
	(void)data;
	(void)link;
	return 0;
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

static int load_fshader(
	void const *filename,
	size_t len,
	void *data,
	void *link)
{
	struct glstate *state = link;
	(void)len;
	return gl_make_shader(state, filename, GL_FRAGMENT_SHADER, data);
}

static int load_vshader(
	void const *filename,
	size_t len,
	void *data,
	void *link)
{
	struct glstate *state = link;
	(void)len;
	return gl_make_shader(state, filename, GL_VERTEX_SHADER, data);
}

static void free_shader(
	void const *filename,
	size_t len,
	void *data,
	void *link)
{
	(void)filename;
	(void)len;
	struct glstate *state = link;
	struct glshader *shader = data;
	state->f.glDeleteShader(shader->name);
}

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

/* Load a glmaterial from a .mtl library file. The key has the form
   "path/to/file.mtl:mtlname". */
static int load_wf_material(void const *key, size_t len, void *data, void *link)
{
	struct glstate *state;
	struct recap cap;
	char *filename, *mtlname;
	struct wf_mtllib const **mtllib;
	struct wf_material const *wfmtl;
	struct glmaterial *mtl;

	(void)len;

	if (!recap("\\(.+\\)$", key, &cap)) { return -1; }
	assert(cap.length > 2);
	state = link;

	filename = strdup_prefix(key, cap.offset);
	mtllib = gl_load_wf_mtllib(&state->cache, filename);
	free(filename);
	if (!mtllib) { return -1; }

	mtlname = strdup_prefix((char *)key + cap.offset + 1, cap.length - 2);
	wfmtl = wf_get_material(*mtllib, mtlname);
	free(mtlname);
	if (!wfmtl) {
		gl_release_wf_mtllib(&state->cache, mtllib);
		return -2;
	}
	mtl = data;
	mtl->diffuse[0] = wfmtl->kd[0];
	gl_release_wf_mtllib(&state->cache, mtllib);
	return 0;
}

static void free_material(void const *key, size_t len, void *data, void *link)
{
	(void)key;
	(void)len;
	(void)data;
	(void)link;
}

static void free_program(void const *key, size_t len, void *data, void *link)
{
	(void)key;
	(void)len;
	(void)data;
	(void)link;
}

static int load_wf_obj(void const *filename, size_t len, void *data, void *link)
{
	struct glgeometries const **p = data;
	struct glstate *state = link;

	(void)len;

	*p = gl_load_wfobj(state, filename);
	return *p ? 0 : -1;
}

static void free_geometry(void const *key, size_t len, void *data, void *link)
{
	struct glgeometries const **geos = data;
	struct glstate *state = link;

	(void)key;
	(void)len;

	gl_free_wfgeo(state, *geos);
}

