
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdalign.h>
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
#include <adt/hmap.h>

#include <glapi/api.h>
#include <glapi/core.h>

#include "include/types.h"
#include "include/cache.h"
#include "private.h"
#include "decl.h"
#include "geometry.h"


static void set_gl_attrib_pointer(
	struct gl_core const *f,
	struct gl_vertexattrib const *va)
{
	f->VertexAttribPointer(
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
	struct gl_cache *cache,
	char const *basepath,
	struct wf_object const *obj)
{
	size_t i, j, namebufsz;
	struct gl_material const **mtllist;
	char *namebuf;
	struct material_map *libmap;

	struct tstack ts;
	jmp_buf errbuf;

	if (setjmp(errbuf)) { return NULL; }
	tstack_init(&ts, &errbuf);

	namebufsz = 1000;
	tstack_push_mem(&ts, namebuf = malloc(namebufsz));
	tstack_push_mem(&ts, mtllist = calloc(obj->ngroups, sizeof *mtllist));

	libmap = make_material_map(cache, basepath, obj->mtllib, obj->nmtllib);
	if (obj->nmtllib > 0) {
		tstack_push(&ts, free_material_map_, libmap, cache);
	}

	for (i = 0; i < obj->ngroups; i++) {
		struct wf_triangles const *group = obj->groups + i;
		if (group->mtlname == NULL) {
			mtllist[i] = gl_default_material(cache);
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
		free_materials(cache, mtllist, i);
		tstack_fail(&ts); /* longjmps away */
	}
	tstack_retain_mem(&ts, mtllist);
	tstack_term(&ts);

	return mtllist;
}

static int push_vertex(
	struct wbuf *vertices,
	float const pos[static 3],
	float const uv[static 2],
	float const norm[static 3],
	GLuint *index)
{
	struct gl_vertex newvtx;
	size_t n;

	n = wbuf_nmemb(vertices, sizeof newvtx);
	(void)memcpy(newvtx.position, pos, sizeof newvtx.position);
	(void)memcpy(newvtx.normal, norm, sizeof newvtx.normal);
	(void)memcpy(newvtx.tcoord, uv, sizeof newvtx.tcoord);

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
	f->GenBuffers(1, &name);
	f->BindBuffer(GL_ARRAY_BUFFER, name);
	f->BufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	/* Associate vertex attributes with VAO */
	for (i = 0; i < nattributes; i++) {
		f->EnableVertexAttribArray(attributes[i].index);
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

	f->GenBuffers(1, &name);
	f->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, name);
	f->BufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
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
	GLuint (*triangle)[3];
	unsigned const (*attr)[3];

	wbuf_init(elements);
	wbuf_init(vertices);

	for (i = 0; i < group->n; i++) {
		triangle = wbuf_alloc(elements, sizeof *triangle);
		if (!triangle) { goto error; }
		for (j = 0; j < 3; j++) {
			attr = &group->indicies[i][j];
			// TODO: reuse vertex if it already exists? No, filter
			// inputs instead and don't optimize here
			err = push_vertex(
				vertices,
				obj->pos[(*attr)[0]],
				obj->uv[(*attr)[1]],
				obj->norm[(*attr)[2]],
				&(*triangle)[j]);
			if (err) { goto error; }
		}
	}
	return 0;

error:	wbuf_term(vertices);
	wbuf_term(elements);
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

static void gl_geometry_term(const void *p, const void *ctx)
{
	struct gl_core const *f = ctx;
	struct gl_geometry const *geo = (void *)p;

	f->DeleteVertexArrays(1, &geo->vao);
	f->DeleteBuffers(2, (GLuint [2]){ geo->vbo, geo->eb.name });
}

static int geometry_init_wf(
	struct gl_api *gl,
	struct gl_geometry *geo,
	struct wf_triangles const *group,
	struct wf_object const *obj)
{
	struct gl_core const *f = gl_get_core(gl);
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

	f->GenVertexArrays(1, &geo->vao);
	f->BindVertexArray(geo->vao);

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

	f->BindVertexArray(0);

	tstack_term(&ts);

	return 0;
}

static int geometires_init_wf(
	struct gl_api *gl,
	struct wf_object const *obj,
	struct gl_material const *const *mtllist,
	struct gl_geometries *geos)
{
	int err;
	size_t i;
	jmp_buf errbuf;
	struct tstack ts;
	struct gl_geometry *geo, *p;

	if (setjmp(errbuf)) { return -1; }
	tstack_init(&ts, &errbuf);

	p = malloc(obj->ngroups * sizeof geos->geo[0]);
	geos->geo = p;
	tstack_push_mem(&ts, geos->geo);
	geos->n = obj->ngroups;
	for (i = 0; i < geos->n; i++) {
		geo = p + i;
		geo->material = mtllist[i];
		err = geometry_init_wf(gl, geo, obj->groups + i, obj);
		if (err) { tstack_fail(&ts); }
		tstack_push(&ts, &gl_geometry_term, geo, gl_get_core(gl));
	}
	tstack_retain_all(&ts);
	tstack_term(&ts);

	return 0;
}

int gl_geometries_init_wfobj(
	struct gl_cache *cache,
	struct gl_geometries *geos,
	char const *filename)
{
	struct wf_object const *obj = NULL;
	struct gl_material const *const *mtllist = NULL;
	int result = -1;

	if (!(obj = wf_parse_object(filename))) { goto clean; }
	if (!(mtllist = load_materials(cache, filename, obj))) { goto clean; }
	result = geometires_init_wf(cache->gl, obj, mtllist, geos);
clean:
	if (mtllist) {
		free_materials(cache, mtllist, obj->ngroups);
		free((void *)mtllist);
	}
	if (obj) wf_free_object(obj);
	return result;
}

void gl_geometries_term(struct gl_cache *cache, struct gl_geometries *geos)
{
	size_t i;
	struct gl_geometry const *geo;

	for (geo = geos->geo, i = 0; i < geos->n; i++, geo++) {
		gl_release_material(cache, geo->material);
		gl_geometry_term(geo, gl_get_core(cache->gl));
	}
	if (geos->geo) {
		free((void *)geos->geo);
	}
}

struct gl_material *gl_default_material(struct gl_cache *cache)
{
	return &cache->defmat;
}

void gl_draw_geometry(struct gl_api *gl, struct gl_geometry const *geo)
{
	struct gl_core const *core = gl_get_core(gl);
	core->BindVertexArray(geo->vao);
	glDrawElements(geo->eb.mode, geo->eb.count, geo->eb.type, 0);
	core->BindVertexArray(0);
}

void gl_draw_geometries(struct gl_api *gl, struct gl_geometries const *geos)
{
	size_t i;
	for (i = 0; i < geos->n; i++) {
		gl_draw_geometry(gl, geos->geo + i);
	}
}

