#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <string.h>

#include <base/mem.h>
#include <glapi/api.h>
#include <glapi/core.h>

#include "include/types.h"
#include "include/shape.h"
#include "private.h"
#include "types.h"

static size_t xylo_count_outlines(size_t n, struct xylo_shape const *shape)
{
	size_t i, m;
	for (m = i = 0; i < n; i++) {
		m += shape->n;
	}
	return m;
}

static size_t xylo_shape_leg_size(struct xylo_shape const *shape)
{
	size_t i, n;
	for (n = i = 0; i < shape->n; i++) {
		n += shape->outlines[i].n;
	}
	return n * sizeof(struct xylo_leg);
}

static GLuint make_shape_set_vbo(
	struct gl_api *api,
	GLenum usage,
	size_t n,
	struct xylo_shape const *shapes)
{
	char *p;
	size_t i, j, sz;
	GLuint vbo;
	struct xylo_shape const *shape;
	struct xylo_outline const *outline;
	struct gl_core33 const *restrict gl;

	for (sz = 0, i = 0; i < n; i++) {
		// FIXME: potential for overflow not handled
		sz += xylo_shape_leg_size(shapes + i);
	}
	gl = gl_get_core33(api);
	gl->GenBuffers(1, &vbo);
	gl->BindBuffer(GL_ARRAY_BUFFER, vbo);
	gl->BufferData(GL_ARRAY_BUFFER, sz, NULL, usage);
	p = gl->MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (p) {
		for (i = 0; i < n; i++) {
			shape = shapes + i;
			for (j = 0; j < shape->n; j++) {
				outline = shape->outlines + j;
				sz = sizeof (*outline->legs) * outline->n;
				(void)memcpy(p, shape->outlines[j].legs, sz);
				p += sz;
			}
		}
		gl->UnmapBuffer(GL_ARRAY_BUFFER);
		gl->BindBuffer(GL_ARRAY_BUFFER, 0);
		return vbo;
	} else {
		gl->DeleteBuffers(1, &vbo);
		return 0;
	}
}

static struct xylo_glshape *alloc_glshapes(
	size_t n,
	struct xylo_shape const *shapes)
{
	size_t i, j, m, glshape_ext, first_ext, size;
	struct xylo_glshape *glshapes, *glshape;
	struct xylo_shape const *shape;
	GLint *firsts, tally;
	GLsizei *counts;

	m = xylo_count_outlines(n, shapes);

	/* Create single block to hold the shapes */
	if (memblk_array(0, &glshape_ext, n, *glshapes)) { return NULL; }
	if (memblk_array(glshape_ext, &first_ext, m, *firsts)) { return NULL; }
	if (memblk_array(first_ext, &size, m, *counts)) { return NULL; }

	if (glshapes = malloc(size), !glshapes) { return NULL; }
	firsts = memblk_offset(glshapes, glshape_ext, *firsts);
	counts = memblk_offset(glshapes, first_ext, *counts);

	glshape = glshapes;
	shape = shapes;
	tally = 0;

	for (i = 0; i < n; i++) {
		glshape->first = firsts;
		glshape->count = counts;
		glshape->drawcount = shape->n;
		for (j = 0; j < shape->n; j++) {
			firsts[j] = tally;
			tally += counts[j] = shape->outlines[j].n;
		}
		firsts += shape->n;
		counts += shape->n;
		glshape++;
		shape++;
	}
	return glshapes;
}

struct xylo_glshape_set *xylo_make_glshape_set(
	struct gl_api *api,
	size_t n,
	struct xylo_shape const *shapes)
{
	static struct {
		GLuint location;
		GLint size;
		ptrdiff_t offset;
	} const attribs[] = {
		{ LINE_POS_ATTRIB, 2, 0 },
		{ WEIGHT_POS_ATTRIB, 2, 2*sizeof(float) },
		{ WEIGHT_VAL_ATTRIB, 1, 4*sizeof(float) }
	};
	struct gl_core33 const *restrict gl;
	size_t i;
	struct xylo_glshape_set *set;
	struct xylo_glshape *glshapes;
	GLuint vao, vbo;
	GLsizei stride;

	gl = gl_get_core33(api);

	glshapes = alloc_glshapes(n, shapes);
	if (!glshapes) { return NULL; }
	vbo = make_shape_set_vbo(api, GL_STATIC_DRAW, n, shapes);
	if (!vbo) {
		free(glshapes);
		return NULL;
	}
	set = malloc(sizeof *set);
	if (!set) {
		gl->DeleteBuffers(1, &vbo);
		free(glshapes);
		return NULL;
	}

	gl->GenVertexArrays(1, &vao);
	gl->BindVertexArray(vao);
	gl->BindBuffer(GL_ARRAY_BUFFER, vbo);

	stride = sizeof(struct xylo_leg);
	for (i = 0; i < length_of(attribs); i++) {
		gl->EnableVertexAttribArray(attribs[i].location);
		gl->VertexAttribPointer(
			attribs[i].location,
			attribs[i].size,
 			GL_FLOAT,
			GL_FALSE,
			stride,
			(GLvoid const *)attribs[i].offset);
	}
	gl->BindBuffer(GL_ARRAY_BUFFER, 0);
	gl->BindVertexArray(0);

	set->vao = vao;
	set->vbo = vbo;
	set->shapes = glshapes;
	set->n = n;

	return set;
}

struct xylo_glshape const *xylo_get_glshape(
	struct xylo_glshape_set *set,
	size_t i)
{
	return set->shapes + i;
}

int xylo_free_glshape_set(struct xylo_glshape_set *set, struct gl_api *api)
{
	struct gl_core33 const *restrict gl = gl_get_core33(api);

	gl->DeleteBuffers(1, &set->vbo);
	gl->DeleteVertexArrays(1, &set->vbo);
	free(set->shapes);
	free(set);
	return 0;
}
