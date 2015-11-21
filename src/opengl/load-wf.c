
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <limits.h>
#include <string.h>
#include <GL/gl.h>

#include <gm/vector.h>
#include <base/mem.h>
#include <base/wbuf.h>
#include <base/tstack.h>
#include <text/str.h>
#include <fs/file.h>
#include <wf/wf.h>

#include "types.h"
#include "decl.h"
#include "load-material.h"
#include "load-mtllib.h"
#include "load-wf.h"

static void set_gl_attrib_pointer(
	struct gl_core const *f,
	struct gl_vertexattrib const *va)
{
	f->glVertexAttribPointer(
		va->index,
		va->size,
		va->type,
		va->normalized,
		va->stride,
		va->pointer);
}

struct material_map
{
	size_t n;
	struct {
		char const *filename;
		struct wf_mtllib const *const *mtl;
	} entries[];
};

static void free_material_map(
	struct gl_cache *cache,
	struct material_map *libmap) 
{
	for (size_t i = libmap->n; i-- > 0; ) {
		if (libmap->entries[i].mtl) {
			gl_release_wf_mtllib(cache, libmap->entries[i].mtl);
		}
		if (libmap->entries[i].filename) {
			free((void *)libmap->entries[i].filename);
		}
	}
}

static void free_material_map_(void const *p, void const *context)
{
	struct material_map *libmap = (struct material_map *)p;
	struct gl_cache *cache = (struct gl_cache *)context;
	free_material_map(cache, libmap);
	free(libmap);
}

static struct material_map *make_material_map(
	struct gl_cache *cache,
	char const *base,
	char const *const *files,
	size_t n) 
{
	struct material_map *libmap;
	size_t i;
	char const *filename;

	if (n == 0) { return NULL; }
	libmap = malloc(sizeof *libmap + sizeof libmap->entries[0] * n);
	if (!libmap) { return NULL; }
	libmap->n = 0;
	for (i = 0; i < n; i++) {
		filename = relpath(base, files[i]);
		if (filename) {
			libmap->entries[i].filename = filename;
			libmap->n++;
			libmap->entries[i].mtl =
				gl_load_wf_mtllib(cache, filename);
			
			if (libmap->entries[i].mtl) continue;
		}
		free_material_map(cache, libmap);
		return NULL;
	}
	return libmap;
}

static void free_materials(
	struct gl_cache *cache,
	struct gl_material const *const *mtllist,
	size_t n)
{
	for (size_t i = n; i-- > 0; ) gl_release_material(cache, mtllist[i]);
}

/* Create an array of materials, one for each group */
static struct gl_material const *const *load_materials(
	struct gl_state *state,
	char const *basepath,
	struct wf_object const *obj)
{
	size_t i, j, namebufsz;
	struct gl_material const **mtllist;
	char *namebuf;
	struct material_map *libmap;
	struct gl_cache *cache;

	struct tstack ts;
	jmp_buf errbuf;

	if (setjmp(errbuf)) { return NULL; }
	tstack_init(&ts, &errbuf);

	namebufsz = 1000;
	tstack_push_mem(&ts, namebuf = malloc(namebufsz));
	tstack_push_mem(&ts, mtllist = calloc(obj->ngroups, sizeof *mtllist));

	cache = &state->cache;
	libmap = make_material_map(cache, basepath, obj->mtllib, obj->nmtllib);
	tstack_push(&ts, free_material_map_, libmap, cache);

	for (i = 0; i < obj->ngroups; i++) {
		struct wf_triangles const *group = obj->groups + i;
		if (group->mtlname == NULL) {
			mtllist[i] = gl_default_material(state);
			continue;
		}
		struct wf_material const *mtl = NULL;
		char const *mtlfilename = NULL;
		for (j = 0; j < obj->nmtllib; j++) {
			mtl = wf_get_material(
				*libmap->entries[j].mtl,
				group->mtlname);
			if (mtl) {
				mtlfilename = libmap->entries[i].filename;
				break;
			}
		}
		if (mtl && mtlfilename) {
			mtllist[i] = gl_load_wf_material(
				cache,
				mtlfilename,
				mtl->name);
			if (mtllist[i]) { continue; }
		}
		/* mtl == NULL || mtlfilename == NULL || mtllist[i] == NULL */
		free_materials(cache, mtllist, i);
		tstack_fail(&ts); /* longjmps away */
	}
	tstack_retain_mem(&ts, mtllist);
	tstack_free(&ts);

	return mtllist;
}

static int get_vertex(
	struct wbuf *vertices,
	float const pos[3],
	float const uv[2],
	float const norm[3],
	GLuint *index)
{
	struct gl_vertex *vtx, newvtx;
	size_t i, n;

	/* Look for an identical old one */
	vtx = vertices->begin;
	n = wbuf_nmemb(vertices, sizeof *vtx);
	for (i = 0; i < n; i++) {
		if (v3neareqf(vtx->position, pos))
		if (v2neareqf(vtx->tcoord, uv))
		if (v3neareqf(vtx->normal, norm)) {
			*index = i;
			return 0;
		}
	}
	/* Make a new one */
	(void)memcpy(newvtx.position, pos, sizeof (float) * 3);
	(void)memcpy(newvtx.normal, norm, sizeof (float) * 3);
	(void)memcpy(newvtx.tcoord, uv, sizeof (float) * 2);
	if (!wbuf_write(vertices, &newvtx, sizeof newvtx)) { return -1; }
	*index = n;
	return 0;
}

static GLuint make_vertex_buffer(
	struct gl_core const *restrict f,
	void *vertices,
	size_t size,
	struct gl_vertexattrib const *attributes,
	size_t nattributes)
{
	GLuint name;
	size_t i;

	/* Make vertex buffer and fill data */
	f->glGenBuffers(1, &name);
	f->glBindBuffer(GL_ARRAY_BUFFER, name);
	f->glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	/* Associate vertex attributes with VAO */
	for (i = 0; i < nattributes; i++) {
		f->glEnableVertexAttribArray(attributes[i].index);
		set_gl_attrib_pointer(f, attributes + i);
	}

	return name;
}

/* Turn an array of GLuint into an array of GLushort or GLubyte, if possible */
static struct element_buffer make_element_buffer(
	struct gl_core const *restrict f,
	GLuint *indicies,
	size_t nmemb,
	GLenum mode,
	size_t max)
{
	size_t i;
	GLuint name;
	GLsizeiptr size;
	GLenum type;
	void *data;
	GLsizei count;

	data = NULL;
	count = nmemb;

	if (max <= UCHAR_MAX) {
		/* Copy to unsigned char buffer */
		size = sizeof (GLubyte) * nmemb;
		GLubyte *cdata = malloc(size);
		if (cdata) {
			for (i = 0; i < nmemb; i++) { cdata[i] = indicies[i]; }
			type = GL_UNSIGNED_BYTE;
			data = cdata;
		}
	} else if (max <= USHRT_MAX) {
		/* Copy to unsigned short buffer */
		size = sizeof (GLushort) * nmemb;
		GLushort *sdata = malloc(size);
		if (sdata) {
			for (i = 0; i < nmemb; i++) { sdata[i] = indicies[i]; }
			type = GL_UNSIGNED_SHORT;
			data = sdata;
		}
	}
	if (data == NULL) {
		type = GL_UNSIGNED_INT;
		size = sizeof (GLuint) * nmemb;
		data = indicies;
	}

	f->glGenBuffers(1, &name);
	f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, name);
	f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	if (data != indicies) { free(data); }

	return (struct element_buffer) { name, count, type, mode };
}

static int make_gl_vertices(
	struct wbuf *vertices,
	struct wbuf *elements,
	struct wf_object const *obj,
	struct wf_triangles const *group)
{
	int err;
	size_t i, j;
	GLuint *index;

	wbuf_init(elements);
	wbuf_init(vertices);

	for (i = 0; i < group->n; i++) {
		index = wbuf_alloc(elements, sizeof (GLuint[3]));
		if (!index) { goto error; }
		for (j = 0; j < 3; j++) {
			err = get_vertex(
				vertices,
				obj->pos[(*group->indicies)[j][0]],
				obj->uv[(*group->indicies)[j][1]],
				obj->norm[(*group->indicies)[j][2]],
				index + j
			);
			if (err) { goto error; }
		}
	}
	return 0;

error:	wbuf_free(vertices);
	wbuf_free(elements);
	return -1;
}

static struct gl_vertexattrib const vertex_attrib[] = {
	{
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct gl_vertex),
		(void *)offsetof(struct gl_vertex, position)
	},
	{
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct gl_vertex),
		(void *)offsetof(struct gl_vertex, normal)
	},
	{
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct gl_vertex),
		(void *)offsetof(struct gl_vertex, tcoord)
	}
};

static void free_wf_geometry(const void *p, const void *ctx)
{
	struct gl_core const *f = ctx;
	struct gl_geometry const *geo = (void *)p;

	f->glDeleteVertexArrays(1, &geo->vao);
	f->glDeleteBuffers(2, (GLuint [2]){ geo->vbo, geo->eb.name });
}

static int make_wf_geometry(
	struct gl_state *state,
	struct gl_geometry *geo,
	struct wf_triangles const *group,
	struct wf_object const *obj)
{
	struct gl_core const *const restrict f = &state->f;
	struct wbuf vertices, elements;
	jmp_buf errbuf;
	struct tstack ts;

	if (setjmp(errbuf)) { return -1; }
	tstack_init(&ts, &errbuf);

	if (make_gl_vertices(&vertices, &elements, obj, group)) {
		tstack_fail(&ts);
	}

	tstack_push_wbuf(&ts, &vertices);
	tstack_push_wbuf(&ts, &elements);

	f->glGenVertexArrays(1, &geo->vao);
	f->glBindVertexArray(geo->vao);

	geo->vbo = make_vertex_buffer(
		f,
		vertices.begin,
		wbuf_size(&vertices),
		vertex_attrib,
		length_of(vertex_attrib));

	geo->eb = make_element_buffer(
		f,
		elements.begin,
		wbuf_nmemb(&elements, sizeof (GLuint)),
		GL_TRIANGLES,
		wbuf_nmemb(&vertices, sizeof (struct gl_vertex)));

	f->glBindVertexArray(0);

	tstack_free(&ts);

	return 0;
}

int make_wf_geometires(
	struct gl_state *state,
	struct wf_object const *obj,
	struct gl_material const *const *mtllist,
	struct gl_geometries *geos)
{
	int err;
	size_t i;
	jmp_buf errbuf;
	struct tstack ts;
	struct gl_geometry *geo;

	if (setjmp(errbuf)) { return -1; }
	tstack_init(&ts, &errbuf);

	geos->geo = malloc(obj->ngroups * sizeof geos->geo[0]);
	tstack_push_mem(&ts, geos->geo);
	geos->n = obj->ngroups;
	for (i = 0; i < geos->n; i++) {
		geo = geos->geo + i;
		geo->material = mtllist[i];
		err = make_wf_geometry(state, geo, obj->groups + i, obj);
		if (err) { tstack_fail(&ts); }
		tstack_push(&ts, &free_wf_geometry, geo, &state->f);
	}
	tstack_retain_all(&ts);
	tstack_free(&ts);

	return 0;
}

int gl_load_wfobj(
	struct gl_state *state,
	struct gl_geometries *geos,
	char const *filename)
{
	struct wf_object const *obj = NULL;
	struct gl_material const *const *mtllist = NULL;
	int result = -1;

	if (!(obj = wf_parse_object(filename))) { goto clean; }
	if (!(mtllist = load_materials(state, filename, obj))) { goto clean; }
	result = make_wf_geometires(state, obj, mtllist, geos);
clean:	
	if (mtllist) {
		free_materials(&state->cache, mtllist, obj->ngroups);
		free((void *)mtllist);
	}
	if (obj) wf_free_object(obj);
	return result;
}

void gl_free_wfgeo(struct gl_state *state, struct gl_geometries *geos)
{
	size_t i;
	struct gl_geometry const *geo;

	for (geo = geos->geo, i = 0; i < geos->n; i++, geo++) {
		gl_release_material(&state->cache, geo->material);
		free_wf_geometry(geo, &state->f);
	}
}

