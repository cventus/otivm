#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <string.h>
#include <math.h>

#include <base/mem.h>
#include <glapi/api.h>
#include <glapi/core.h>

#include "include/types.h"
#include "include/shape.h"
#include "private.h"
#include "types.h"

#define VERTEX_FLOATS 5

static size_t vertex_size(size_t n)
{
	return n * sizeof(float) * VERTEX_FLOATS;
}

static size_t vertex_count(struct xylo_shape const *shape)
{
	size_t i, n;
	for (n = i = 0; i < shape->n; i++) {
		n += shape->outlines[i].n * 3;
	}
	return n;
}

static size_t max_vertex_count(size_t n, struct xylo_shape const *shapes)
{
	size_t i, m, max;
	for (max = i = 0; i < n; i++) {
		m = vertex_count(shapes + i);
		if (max < m) { max = m; }
	}
	return max;
}

static GLenum index_type(size_t max_index)
{
	if (max_index < 256) {
		return GL_UNSIGNED_BYTE;
	} else if (max_index < 65536) {
		return GL_UNSIGNED_SHORT;
	} else {
		return GL_UNSIGNED_INT;
	}
}

static size_t type_size(GLenum type)
{
	switch (type) {
	case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
	case GL_UNSIGNED_SHORT: return sizeof(GLushort);
	case GL_UNSIGNED_INT: return sizeof(GLuint);
	default: assert(!"unknown type");
	}
}

/* allocate space for buffer meta-data */
static struct xylo_glshape_set *alloc_block(size_t n)
{
	struct memblk blk[2];
	struct xylo_glshape_set *set;

	/* create single block to hold the shapes and set header */
	if (memblk_init(blk+0, 1, sizeof(*set))) { return NULL; }
	if (memblk_push(blk+1, n, sizeof(*set->shapes), alignof(*set->shapes))) {
		return NULL;
	}

	if (set = malloc(blk[1].extent), !set) { return NULL; }
	set->shapes = memblk_offset(set, blk[1]);
	set->n = n;
	set->vao = set->evbo = 0;

	return set;
}

static float *append_center(float *dest, struct xylo_shape const *shape)
{
	static float const q0[3] = { 0.f, 1.f, 1.f };

	size_t i, j, n;
	struct xylo_outline const *outline;
	struct xylo_leg const *leg;

	dest[0] = dest[1] = 0.f;
	n = 0;
	for (i = 0; i < shape->n; i++) {
		outline = shape->outlines + i;
		n += outline->n;
		for (j 	= 0; j < outline->n; j++) {
			leg = outline->legs + j;
			dest[0] += leg->end[0];
			dest[1] += leg->end[1];
		}
	}

	/* Shape space */
	dest[0] /= n;
	dest[1] /= n;
	(void)memcpy(dest + 2, q0, sizeof q0);
	return dest + VERTEX_FLOATS;
}

static void wcoord(float *dest, float w)
{
	float winv = (w <= 0.f) ? HUGE_VAL : 1.f / w;

	dest[0] = 0.5f * winv;
	dest[1] = 0.0f;
	dest[2] = winv;
}

/* Append outline vertices to `dest`, where each segment's vertices (on-curve
   and off-curve pair) in the order 0) off-curve, 1) on-curve, 2) on-curve from
   the following segment. Each vertex contains two attributes: 2D-position and
   quadratic-curve space position. */
static float *append_vertices(float *dest, struct xylo_outline const *outline)
{
	/* TODO: find more compact representation of quadratic coordinates */
	static float const q1[3] = { 0.f, 0.f, 1.f };
	static float const q2[3] = { 1.f, 1.f, 1.f };

	size_t i;
	struct xylo_leg const *leg;

	/* add three vertices per triangle side */
	leg = outline->legs;
	for (i = 0; i < outline->n; i++) {
		/* weighted control point */
		(void)memcpy(dest, leg->mid, sizeof leg->mid);
		wcoord(dest + 2, leg->weight);

		/* on-curve control point*/
		(void)memcpy(dest + 5, leg->end, sizeof leg->end);
		(void)memcpy(dest + 7, q1, sizeof q1);

		/* next on-curve control point */
		leg = outline->legs + (i + 1 == outline->n ? 0 : i + 1);
		(void)memcpy(dest + 10, leg->end, sizeof leg->end);
		(void)memcpy(dest + 12, q2, sizeof q2);
		dest += 3*VERTEX_FLOATS;
	}
	return dest;
}

/* Create a single index large enough to support the shape with the most
   vertices.

   Each outline segment (on-curve point and off-curve point pair) together with
   their following on-curve vertex are turned into two triangles: one extending
   from the common center point to the on-curve points, and one between the
   three points. The former triangle is part of a triangle fan that trace the
   outline and the latter triangle specifies curvature at the segment. Each
   segment results in three vertices, and thus in combination with a shared
   center vertex the maximum index is a multiple of three. Due to the way
   vertices can be shared, the number of indices is twice that.

   The same indices can be used for all shapes. The indicies that define the
   two sets of triangles can grow from the middle towards the beginning and
   end. The common center point has index 0, the first segment has vertices 1,
   2, and 3, and so on. I.e. an index that could be used for shapes with two or
   three segments could both use the following index (lines breaks added for
   legibility):

     9 8 0
     6 5 0
     3 2 0
     1 2 3
     4 5 6
     7 8 9

   Shapes with two segments would use indices { 6, 5, 0, 3, ..., 4, 5, 6 } and
   shapes with three segments would use all { 9, 8, 0, ..., 7, 8, 9 }. When
   drawing you only need to provide the proper `indices` offset. */
static void append_indices(void *dest, size_t max_index, GLenum type)
{
	size_t i;

#define define_index(type) {\
	type *p = dest; \
	for (i = max_index; i > 1; ) { \
		/* p2 -> p1 -> p0 */ \
		*p++ = i--; \
		*p++ = i--; \
		*p++ = (i--, 0); \
	} \
	for (i = 1; i <= max_index; i++) { \
		/* w -> p1 -> p2 */ \
		*p++ = i; \
	} \
}

	switch (type) {
	case GL_UNSIGNED_BYTE:
		define_index(GLubyte);
		break;
	case GL_UNSIGNED_SHORT:
		define_index(GLushort);
		break;
	case GL_UNSIGNED_INT:
		define_index(GLuint);
		break;
	default:
		assert(!"unknown type");
	}
#undef define_index
}

/* Set up basevertex offsets, index offsets, and all other parameters needed
   for drawing for each shape and return the total number of vertices. */
static size_t setup_parameters(
	size_t max_index,
	GLenum type,
	size_t n,
	struct xylo_shape const *shapes,
	struct xylo_glshape *glshapes)
{
	size_t i, vertices, shape_vertices, offset;

	vertices = 0;
	for (i = 0; i < n; i++) {
		glshapes[i].type = type;
		glshapes[i].basevertex = vertices;
		shape_vertices = vertex_count(shapes + i);
		offset = max_index - shape_vertices;
		glshapes[i].indices = (void const *)offset;
		glshapes[i].count = shape_vertices * 2;
		vertices += shape_vertices + 1;
	}

	return vertices;
}

/* Copy shape data into GPU buffers and drawing meta-data into `set`. */
static GLuint make_shape_buffers(
	struct gl_api *api,
	GLenum usage,
	size_t n,
	struct xylo_shape const *shapes,
	struct xylo_glshape_set *set)
{
	void *p;
	size_t i, j, vertices, max_index, index_size, buffer_size;
	struct xylo_shape const *shape;
	struct gl_core33 const *restrict gl;
	GLuint evbo;
	GLenum type;

	// FIXME: potential for overflow not handled
	max_index = max_vertex_count(n, shapes);
	type = index_type(max_index);
	index_size = align_to(type_size(type) * max_index * 2, sizeof(float));

	vertices = setup_parameters(max_index, type, n, shapes, set->shapes);

	gl = gl_get_core33(api);
	gl->GenBuffers(1, &evbo);
	gl->BindBuffer(GL_ARRAY_BUFFER, evbo);

	set->vertex_offset = index_size;
	buffer_size = index_size + vertex_size(vertices);

	gl->BufferData(GL_ARRAY_BUFFER, buffer_size, NULL, usage);
	do {
		p = gl->MapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
		if (!p) {
			gl->DeleteBuffers(1, &evbo);
			return 0;
		}
		append_indices(p, max_index, type);
		p = (char *)p + index_size;
		for (i = 0; i < n; i++) {
			shape = shapes + i;
			p = append_center(p, shape);
			for (j = 0; j < shape->n; j++) {
				p = append_vertices(p, shape->outlines + j);
			}
		}
		// FIXME: infinite loop?
	} while (gl->UnmapBuffer(GL_ARRAY_BUFFER) == GL_FALSE);
	gl->BindBuffer(GL_ARRAY_BUFFER, 0);

	return evbo;
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
		{ ATTRIB_SHAPE_POS, 2, 0 },
		{ ATTRIB_QUADRATIC_POS, 3, 2*sizeof(float) }
	};

	struct gl_core33 const *restrict gl;
	size_t i;
	struct xylo_glshape_set *set;
	GLuint vao, evbo;
	GLsizei stride;

	gl = gl_get_core33(api);

	set = alloc_block(n);
	if (!set) { return NULL; }
	evbo = make_shape_buffers(api, GL_STATIC_DRAW, n, shapes, set);
	if (evbo == 0) {
		free(set);
		return NULL;
	}
	gl->GenVertexArrays(1, &vao);
	gl->BindVertexArray(vao);
	gl->BindBuffer(GL_ARRAY_BUFFER, evbo);
	gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, evbo);

	stride = sizeof(float) * VERTEX_FLOATS;
	for (i = 0; i < length_of(attribs); i++) {
		gl->EnableVertexAttribArray(attribs[i].location);
		gl->VertexAttribPointer(
			attribs[i].location,
			attribs[i].size,
 			GL_FLOAT,
			GL_FALSE,
			stride,
			(GLvoid const *)(attribs[i].offset+set->vertex_offset));
	}
	gl->BindVertexArray(0);
	gl->BindBuffer(GL_ARRAY_BUFFER, 0);
	gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	set->vao = vao;
	set->evbo = evbo;

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

	gl->DeleteBuffers(1, &set->evbo);
	gl->DeleteVertexArrays(1, &set->vao);
	free(set);
	return 0;
}
