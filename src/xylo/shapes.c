
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <string.h>
#include <GL/gl.h>
#include <base/mem.h>
#include <glapi/api.h>
#include <glapi/core.h>

#include "include/types.h"
#include "private.h"
#include "types.h"
#include "attributes.h"

static size_t count_outlines(size_t n, struct xylo_shape const shape[n])
{
	size_t i, m;
	for (m = i = 0; i < n; i++) {
		m += shape->n;
	}
	return m;
}

static size_t shape_leg_size(struct xylo_shape const *shape)
{
	size_t i, n;
	for (n = i = 0; i < shape->n; i++) {
		n += shape->outlines[i].n;
	}
	return n * sizeof(struct xylo_leg);
}

static GLuint make_shape_set_vbo(
	struct gl_api *gl,
	GLenum usage,
	size_t n,
	struct xylo_shape const shapes[n])
{
	char *p;
	size_t i, j, sz;
	GLuint vbo;
	struct xylo_shape const *shape;
	struct xylo_outline const *outline;
	struct gl_core const *restrict core;

	for (sz = 0, i = 0; i < n; i++) {
		// FIXME: potential for overflow not handled
		sz += shape_leg_size(shapes + i);
	}
	core = gl_get_core(gl);
	core->GenBuffers(1, &vbo);
	core->BindBuffer(GL_ARRAY_BUFFER, vbo);
	core->BufferData(GL_ARRAY_BUFFER, sz, NULL, usage);
	p = core->MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
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
		core->UnmapBuffer(GL_ARRAY_BUFFER);
		core->BindBuffer(GL_ARRAY_BUFFER, 0);
		return vbo;
	} else {
		core->DeleteBuffers(1, &vbo);
		return 0;
	}
}

static struct xylo_glshape *alloc_glshapes(
	size_t n,
	struct xylo_shape const shapes[n])
{
	size_t i, j, m, glshape_ext, first_ext, size;
	struct xylo_glshape *glshapes, *glshape;
	struct xylo_shape const *shape;
	GLint *firsts, tally;
	GLsizei *counts;

	m = count_outlines(n, shapes);

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
	struct gl_api *gl,
	size_t n,
	struct xylo_shape const shapes[n])
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
	struct gl_core const *restrict core;
	size_t i;
	struct xylo_glshape_set *set;
	struct xylo_glshape *glshapes;
	GLuint vao, vbo;
	GLsizei stride;

	core = gl_get_core(gl);

	glshapes = alloc_glshapes(n, shapes);
	if (!glshapes) { return NULL; }
	vbo = make_shape_set_vbo(gl, GL_STATIC_DRAW, n, shapes);
	if (!vbo) {
		free(glshapes);
		return NULL;
	}
	set = malloc(sizeof *set);
	if (!set) {
		core->DeleteBuffers(1, &vbo);
		free(glshapes);
		return NULL;
	}

	core->GenVertexArrays(1, &vao);
	core->BindVertexArray(vao);
	core->BindBuffer(GL_ARRAY_BUFFER, vbo);

	stride = sizeof(struct xylo_leg);
	for (i = 0; i < length_of(attribs); i++) {
		core->EnableVertexAttribArray(attribs[i].location);
		core->VertexAttribPointer(
			attribs[i].location,
			attribs[i].size,
 			GL_FLOAT,
			GL_FALSE,
			stride,
			(GLvoid const *)attribs[i].offset);
	}
	core->BindBuffer(GL_ARRAY_BUFFER, 0);
	core->BindVertexArray(0);

	set->vao = vao;
	set->vbo = vbo;
	set->shapes = glshapes;
	set->n = n;

	return set;
}

struct xylo_glshape *xylo_get_glshape(struct xylo_glshape_set *set, size_t i)
{
	return set->shapes + i;
}

int xylo_free_glshape_set(struct xylo_glshape_set *set, struct gl_api *gl)
{
	struct gl_core const *restrict core = gl_get_core(gl);

	core->DeleteBuffers(1, &set->vbo);
	core->DeleteVertexArrays(1, &set->vbo);
	free(set->shapes);
	free(set);
	return 0;
}

/*
void xylo_glshape_set_leg(
	struct gl_api *gl,
	struct xylo_glshape *glshape,
	struct xylo_shape const *shapes)
{
	char *p;
	size_t i, j, sz;
	GLuint vbo;
	struct xylo_shape const *shape;
	struct xylo_outline const *outline;
	struct gl_core const *restrict core;

	core = gl_get_core(gl);
	core->GenBuffers(1, &vbo);
	core->BindBuffer(GL_ARRAY_BUFFER, vbo);
	core->BufferData(GL_ARRAY_BUFFER, sz, NULL, GL_STATIC_DRAW);
	p = core->MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
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
		core->UnmapBuffer(GL_ARRAY_BUFFER);
		core->BindBuffer(GL_ARRAY_BUFFER, 0);
		return vbo;
	} else {
		core->DeleteBuffers(1, &vbo);
		return 0;
	}
}
*/
