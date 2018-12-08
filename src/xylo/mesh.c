#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "base/mem.h"
#include "base/wbuf.h"
#include "glapi/api.h"
#include "glapi/core.h"
#include "spline/triangulate.h"
#include "spline/shape.h"
#include "xylo/types.h"
#include "xylo/shape.h"

#include "mesh.h"
#include "private.h"
#include "xylo.h"
#include "types.h"

/* When points are close enough to be considered the same */
#define MIN_SQUARED_DISTANCE 1e-10

/* a triangle containing a curve */
struct curve
{
	unsigned key[3], indices[3];
	float weight;
};

static void swap_gt(unsigned *a, unsigned *b)
{
	unsigned tmp;
	if (*a > *b) {
		tmp = *a;
		*a = *b;
		*b = tmp;
	}
}

/* a key of a triangle containg a curve is its sorted triangle indices */
static void init_curve_key(unsigned dest[3], unsigned src[3])
{
	(void)memcpy(dest, src, sizeof(unsigned[3]));
	swap_gt(dest + 0, dest + 1);
	swap_gt(dest + 0, dest + 2);
	swap_gt(dest + 1, dest + 2);
}

static int uintcmp(unsigned a, unsigned b)
{
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

/* for bsearch */
static int curve_key_cmp(void const *a, void const *b)
{
	int res;
	unsigned const *l = *(unsigned const (*)[3])a;
	unsigned const *r = ((struct curve const *)b)->key;

	res = uintcmp(l[0], r[0]);
	if (res) return res;
	res = uintcmp(l[1], r[1]);
	if (res) return res;
	return uintcmp(l[2], r[2]);
}

/* for qsort */
static int curve_cmp(void const *a, void const *b)
{
	return curve_key_cmp(((struct curve const *)a)->key, b);
}

static void barycenter(
	float dest[2],
	unsigned indices[3],
	float (*vertices)[2])
{
	unsigned a, b, c;

	a = indices[0];
	b = indices[1];
	c = indices[2];

	dest[0] = (vertices[a][0] + vertices[b][0] + vertices[c][0]) / 3.0;
	dest[1] = (vertices[a][1] + vertices[b][1] + vertices[c][1]) / 3.0;
}

/* inside by even-odd rule */
static bool is_inside(
	unsigned (*bounds)[2],
	size_t n,
	float (*points)[2],
	float point[2])
{
	enum { X, Y };
	bool inside;
	size_t i;
	float x, y, dy, dx, m, b;
	float (*org)[2], (*dest)[2];

	
	inside = false;
	x = X[point];
	y = Y[point];

	// fprintf(stderr, " --- --- \n\n");
	// fprintf(stderr, "%g\t%g\n", -4.0, y);
	// fprintf(stderr, "%g\t%g\n", x, y);
	// fprintf(stderr, " --- --- \n\n");

	for (i = 0; i < n; i++) {
		org = points + bounds[i][0];
		dest = points + bounds[i][1];

		/* only consider crossing edges*/
		if ((Y[*org] > y) == (Y[*dest] > y)) { continue; }
		if ((X[*org] > x) && (X[*dest] > x)) { continue; }

		/* deal with vertical lines etc. early */
		if ((X[*org] <= x) && (X[*dest] <= x)) {
			inside = !inside;
			continue;
		}

		/* is (x, y) under the line? */
		dx = X[*dest] - X[*org];
		dy = Y[*dest] - Y[*org];

		if (dx < 0.f) {
			dx = -dx;
			dy = -dy;
		}

		m = dy / dx;
		b = - X[*dest] * m + Y[*dest];

		// fprintf(stderr, "m = %g; ", m);
		// fprintf(stderr, "b = %g\n\n", b);

		if (dy < 0.0) {
			if (x * m + b < y) {
				inside = !inside;
			}
		} else {
			if (x * m + b > y) {
				inside = !inside;
			}
		}

		// if ((x - X[*dest]) * dy < (y - Y[*dest]) * dx) {
			// inside = !inside;
		// }
	}
	// fprintf(stderr, " %s\n", inside ? "INSIDE" : "OUTSIDE");
	// fprintf(stderr, " --- --- \n");
	// fprintf(stderr, " --- --- \n");
	return inside;
}

static float square_distance(float const a[2], float const b[2])
{
	double dx, dy;
	dx = a[0] - b[0];
	dy = a[1] - b[1];
	return dx*dx + dy*dy;
}

static float (*insert_vertex(float (*v)[2], size_t *n, float const pt[2]))[2]
{
	size_t i, m;
	for (i = 0, m = *n; i < m; i++) {
		if (square_distance(v[i], pt) < MIN_SQUARED_DISTANCE) {
			return v + i;
		}
	}
	++*n;
	return memcpy(v[i], pt, sizeof *v);
}

static void insert_edge(unsigned edge[2], unsigned a, unsigned b)
{
	edge[0] = a;
	edge[1] = b;
}

static void write_triangle(
	struct wbuf *buf,
	unsigned indices[3],
	float weight,
	bool invert,
	float (*vertices)[2])
{
	float q[3][3], one, winv, s;
	float (*p)[2];
	size_t i;

	s = invert ? -1.0 : 1.0;
	one = copysignf(1.0f, s);
	winv = copysignf((weight <= 0.f) ? INFINITY : 1.f / weight, s);

	q[0][0] = 0.f;
	q[0][1] = 0.f;
	q[0][2] = one;

	q[1][0] = 0.5f * winv;
	q[1][1] = 0.0f;
	q[1][2] = winv;

	q[2][0] = one;
	q[2][1] = one;
	q[2][2] = one;

	for (i = 0; i < 3; i++) {
		p = vertices + indices[i];
		(void)wbuf_write(buf, *p, sizeof *p);
		(void)wbuf_write(buf, q[i], sizeof q[i]);
	}
}

static void write_fill(
	struct wbuf *buf,
	unsigned indices[3],
	float (*vertices)[2])
{
	static float const q[3][3] = {
		{ 0.f, 0.f, 1.f },
		{ 0.f, 1.f, 1.f },
		{ 1.f, 1.f, 1.f }
	};

	float (*p)[2];
	size_t i;

	for (i = 0; i < 3; i++) {
		p = vertices + indices[i];
		(void)wbuf_write(buf, *p, sizeof *p);
		(void)wbuf_write(buf, q[1], sizeof q[1]);
	}
}

static size_t vertex_size(size_t n)
{
	return n * sizeof(float) * VERTEX_FLOATS;
}

static int push_triangulated_shape(
	struct wbuf *buf,
	struct spline_shape const *shape)
{
	typedef float float2[2];
	typedef unsigned edge[2];

	struct memblk blk[3];
	struct spline_outline const *outline;
	size_t i, j, k, nseg, nv, ne, nc;
	float2 *v, *p0, *p1, *p2, pt;
	edge *e, *ebound, *ecurve;
	bool inside;
	unsigned key[3], (*tri)[3];
	struct curve *curve, *c;
	int nwritten;
	struct triangle_set *triangles;
	float weight;

	/* count number of segments */
	nseg = 0;
	for (i = 0; i < shape->n; i++) {
		outline = shape->outlines + i;
		nseg += outline->n;
	}

	/* Allocate temporary storage */
	if (memblk_init(blk+0, nseg * 2, sizeof(*v))) return 0;
	if (memblk_push(blk+1, nseg * 3, sizeof(*e), alignof(*e))) return 0;
	if (memblk_push(blk+2, nseg, sizeof(*c), alignof(*c))) return 0;

	if (v = malloc(blk[2].extent), !v) { return 0; }
	e = memblk_offset(v, blk[1]);
	c = memblk_offset(v, blk[2]);

	nc = 0;
	ne = nseg * 3;
	nv = 0;

	ebound = e + nseg;
	ecurve = e + nseg;

	/* remove duplicate vertices */
	for (i = 0; i < shape->n; i++) {
		outline = shape->outlines + i;
		for (j = 0; j < outline->n; j++) {
			k = (j + 1) % outline->n;

			p0 = insert_vertex(v, &nv, outline->segments[j].end);
			p2 = insert_vertex(v, &nv, outline->segments[k].end);

			if (p0 != p2) {
				insert_edge(*--ebound, p0 - v, p2 - v);
				p1 = insert_vertex(v, &nv, outline->segments[j].mid);
				if (p1 != p0 && p1 != p2) {
					insert_edge(*ecurve++, p0 - v, p1 - v);
					insert_edge(*ecurve++, p1 - v, p2 - v);

					c[nc].indices[0] = p0 - v;
					c[nc].indices[1] = p1 - v;
					c[nc].indices[2] = p2 - v;

					init_curve_key(c[nc].key, c[nc].indices);
					c[nc].weight = outline->segments[j].weight;
					nc++;
				}
			}
		}
	}

	/* triangulate shape */
	nwritten = 0;
	ne = ecurve - ebound;
	triangles = triangle_set_triangulate(
		(float const (*)[2])v, nv,
		(unsigned const (*)[2])ebound, ne);
	if (!triangles) { goto ret; }
	if (wbuf_reserve(buf, vertex_size(triangles->n * 3))) { goto ret; }

	/* check which triangles are inside/outside */
	qsort(c, nc, sizeof *c, curve_cmp);
	for (i = 0; i < triangles->n; i++) {
		tri = triangles->indices + i;
		barycenter(pt, *tri, v);
		inside = is_inside(ebound, nseg, v, pt);
		init_curve_key(key, *tri);
		curve = bsearch(key, c, nc, sizeof *c, curve_key_cmp);
		if (curve) {
			tri = &curve->indices;
			weight = curve->weight;
			write_triangle(buf, *tri, weight, curve && inside, v);
			nwritten++;
		} else if (inside) {
			write_fill(buf, *tri, v);
			nwritten++;
		} else {
			/* outside non-curve triangle */
		}
	}

ret:	free(triangles);
	free(v);
	return nwritten;
}

/* allocate single memory block for buffer meta-data */
static struct xylo_mesh_set *make_block(size_t n)
{
	struct memblk blk[2];
	struct xylo_mesh_set *set;

	/* create single block to hold the shapes and set header */
	if (memblk_init(blk+0, 1, sizeof(*set))) { return NULL; }
	if (memblk_push(blk+1, n, sizeof(*set->shapes), alignof(*set->shapes))) {
		return NULL;
	}
	if (set = malloc(blk[1].extent), !set) { return NULL; }
	set->shapes = memblk_offset(set, blk[1]);
	set->n = n;

	return set;
}

struct xylo_mesh_set *xylo_make_mesh_set(
	struct gl_api *api,
	size_t n,
	struct spline_shape const *shapes)
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
	size_t i, acc, sz;
	struct xylo_mesh_set *set;
	GLsizei stride;
	struct wbuf buf;
	int m;
	struct spline_shape *simple;

	set = make_block(n);
	if (!set) { return NULL; }
	gl = gl_get_core33(api);
	gl->GenBuffers(1, &set->vbo);
	gl->GenVertexArrays(1, &set->vao);
	wbuf_init(&buf);

	acc = 0;
	for (i = 0; i < n; i++) {
		simple = spline_simplify_shape(shapes + i);
		if (!simple) {
			xylo_free_mesh_set(set, api);
			wbuf_term(&buf);
			return NULL;
		}
		m = push_triangulated_shape(&buf, simple);
		spline_free_shape(simple);
		if (m < 0) { 
			xylo_free_mesh_set(set, api);
			wbuf_term(&buf);
			return NULL;
		}
		m *= 3;
		set->shapes[i].first = acc;
		set->shapes[i].count = m;
		set->shapes[i].vao = set->vao;
		acc += m;
	}

	/* copy buffer to GPU */
	sz = wbuf_size(&buf);
	gl->BindVertexArray(set->vao);
	gl->BindBuffer(GL_ARRAY_BUFFER, set->vbo);
	gl->BufferData(GL_ARRAY_BUFFER, sz, buf.begin, GL_STATIC_DRAW);
	wbuf_term(&buf);

	stride = sizeof(float) * VERTEX_FLOATS;
	for (i = 0; i < length_of(attribs); i++) {
		gl->EnableVertexAttribArray(attribs[i].location);
		gl->VertexAttribPointer(
			attribs[i].location,
			attribs[i].size,
 			GL_FLOAT,
			GL_FALSE,
			stride,
			(GLvoid const *)(attribs[i].offset));
	}
	gl->BindVertexArray(0);
	gl->BindBuffer(GL_ARRAY_BUFFER, 0);

	return set;
}

void xylo_free_mesh_set(struct xylo_mesh_set *set, struct gl_api *api)
{
	struct gl_core33 const *restrict gl;
	gl = gl_get_core33(api);
	gl->DeleteBuffers(1, &set->vbo);
	gl->DeleteVertexArrays(1, &set->vao);
	free(set);
}

struct xylo_mesh const *xylo_get_mesh(
	struct xylo_mesh_set *set,
	size_t i)
{
	return set->shapes + i;
}

void xylo_mesh_draw(
       struct gl_core33 const *restrict gl,
       struct xylo_mesh const *mesh,
       GLsizei samples)
{
	int stencil_test;
	assert(xylo_get_uint(gl, GL_VERTEX_ARRAY_BINDING) == mesh->vao);
	stencil_test = gl->IsEnabled(GL_STENCIL_TEST);
	if (stencil_test) { gl->Disable(GL_STENCIL_TEST); }
	gl->DrawArraysInstanced(
		GL_TRIANGLES,
		mesh->first,
		mesh->count,
		samples);
	if (stencil_test) { gl->Enable(GL_STENCIL_TEST); }
}
