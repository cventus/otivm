#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdalign.h>

#include "base/wbuf.h"
#include "base/mem.h"
#include "spline/triangulate.h"

#include "geometry.h"
#include "quadedge.h"
#include "bits.h"

typedef unsigned triangle_indices[3];

static bool is_right_of(struct eset *set, float2 p, eref e)
{
	return is_ccw(p, **dest(set, e), **org(set, e));
}

static bool is_left_of(struct eset *set, float2 p, eref e)
{
	return is_ccw(p, **org(set, e), **dest(set, e));
}

static bool is_valid(struct eset *set, eref e, eref basel)
{
	return is_right_of(set, **dest(set, e), basel);
}

/* triangulate the simple polygon on the left of s, time complexity in O(n²) */
static int triangulate_polygon(struct eset *set, eref s)
{
	int count, res;
	eref a, b, c, d, e, end;
	float2 *p0, *p1, *p2, *p;
	double max_dist, dist;
	struct line2d perim[3];

	a = s;
	b = lnext(set, a);
	if (lnext(set, b) == lprev(set, a)) {
		/* already a tringle */
		return 0;
	}
	p0 = *org(set, a);
	p1 = *dest(set, a);
	p2 = *dest(set, b);
	/* find the first angle < 180 degrees */
	while (!is_ccw(*p0, *p1, *p2)) {
		/* is the right face is the outside of a convex polygon? */
		if (b == s) { return -1; }
		a = b;
		b = lnext(set, b);
		p0 = p1;
		p1 = p2;
		p2 = *dest(set, b);
	}
	if (eset_alloc(set, 1, &e)) { return -2; }
	/* find diagonal or create an edge from dest(a) to org(b) */
	perim[0] = make_line2d(*p1, *p0);
	perim[1] = make_line2d(*p2, *p1);
	perim[2] = make_line2d(*p0, *p2);
	max_dist = -1.0;
	c = b;
	end = lprev(set, a);
	while (c = lnext(set, c), c != end) {
		p = *dest(set, c);
		if (point2d_in_triangle(perim, *p)) {
			/* candidate found */
			dist = line2d_dist(perim[2], *p);
			if (dist > max_dist) {
				max_dist = dist;
				d = c;
			}
		}
	}
	count = 1;
	if (max_dist > -1.0) {
		/* add diagonal from origin of d to origin of a, and triangulate
		   both sides of the rest of the polygon */
		eset_connect(set, d, b, e);
		res = triangulate_polygon(set, e);
		if (res >= 0) {
			count += res;
			res = triangulate_polygon(set, sym(e));
		}
	} else {
		/* add new edge e, forming triangle a->e->b->a */
		eset_connect(set, b, a, e);

		/* triangulate the rest of the polygon */
		res = triangulate_polygon(set, sym(e));
	}
	if (res < 0) {
		eset_delete(set, e);
		count = -1;
	} else {
		count += res;
	}
	return count;
}

static eref select_cand(
	struct eset *set,
	eref basel,
	eref init,
	eref next(struct eset *, eref),
	int *count)
{
	eref cand, tmp;

	cand = next(set, init);
	if (is_valid(set, cand, basel)) {
		while (point2d_in_circle(
			**dest(set, basel),
			**org(set, basel),
			**dest(set, cand),
			**dest(set, next(set, cand)))
		) {
			tmp = next(set, cand);
			eset_delete(set, cand);
			(*count)--;
			cand = tmp;
		}
	}
	return cand;
}

/* implementation of Guibas & Stolfi (1985) */
static int delauney(
	struct eset *set,
	float2 **verts,
	size_t nmemb,
	eref *le,
	eref *re)
{
	int count;
	assert(nmemb >= 2);
	if (nmemb == 2) {
		/* a single edge connecting the points */
		eref a;
		if (eset_alloc(set, 1, &a)) { return -1; }
		*org(set, a) = verts[0];
		*dest(set, a) = verts[1];
		*le = a;
		*re = sym(a);
		return 1;
	} else if (nmemb == 3) {
		/* A = 0 -> 1, and B = 1 -> 2 */
		eref a, b, c;
		if (eset_alloc(set, 2, &a, &b)) { return -1; }
		eset_splice(set, sym(a), b);
		*org(set, a) = verts[0];
		*dest(set, a) = *org(set, b) = verts[1];
		*dest(set, b) = verts[2];
		if (is_ccw(*verts[0], *verts[2], *verts[1])) {
			if (eset_alloc(set, 1, &c)) { return -1; }
			eset_connect(set, b, a, c);
			*le = sym(c);
			*re = c;
			return 3;
		} else {
			count = 2;
			if (is_ccw(*verts[0], *verts[1], *verts[2])) {
				if (eset_alloc(set, 1, &c)) { return -1; }
				count++;
				eset_connect(set, b, a, c);
			} /* else, co-linear points */
			*le = a;
			*re = sym(b);
			return count;
		}
	} else {
		int lcount, rcount;
		size_t ln, rn;
		eref ldo, ldi, rdi, rdo, basel, lcand, rcand, tmp;
		bool lvalid, rvalid;

		ln = nmemb / 2;
		rn = nmemb - ln;
		lcount =  delauney(set, verts, ln, &ldo, &ldi);
		if (lcount < 0) { return -1; }
		rcount = delauney(set, verts + ln, rn, &rdi, &rdo);
		if (rcount < 0) { return -1; }
		count = lcount + rcount;

		/* find lower common tangent of L and R */
		while (1) {
			if (is_left_of(set, **org(set, rdi), ldi)) {
				ldi = lnext(set, ldi);
			} else if (is_right_of(set, **org(set, ldi), rdi)) {
				rdi = rprev(set, rdi);
			} else break;
		}

		/* connect left and right subdivisions */
		if (eset_alloc(set, 1, &basel)) { return -1; }
		eset_connect(set, sym(rdi), ldi, basel);
		count++;
		if (*org(set, ldi) == *org(set, ldo)) { ldo = sym(basel); }
		if (*org(set, rdi) == *org(set, rdo)) { rdo = basel; }

		/* proceed by merging upwards */
		do {
			lcand = select_cand(set, basel, sym(basel), onext, &count);
			rcand = select_cand(set, basel, basel, oprev, &count);

			/* are we done? */
			lvalid = is_valid(set, lcand, basel);
			rvalid = is_valid(set, rcand, basel);
			if (!lvalid && !rvalid) { break; }

			/* else add cross edge */
			if (eset_alloc(set, 1, &tmp)) { return -1; }
			count++;
			if (!lvalid ||
			    (rvalid && point2d_in_circle(**dest(set, lcand),
			                                 **org(set, lcand),
			                                 **org(set, rcand),
			                                 **dest(set, rcand)))) {
				eset_connect(set, rcand, sym(basel), tmp);
			} else {
				eset_connect(set, sym(basel), sym(lcand), tmp);
			}
			/* continue building from it */
			basel = tmp;
		} while (1);
		*le = ldo;
		*re = rdo;
		return count;
	}
}

static void mark_orbit(
	void *mark,
	struct eset *set,
	eref e,
	eref next(struct eset *, eref))
{
	eref a = e;
	do {
		bits_set(mark, a >> 1);
		a = next(set, a);
	} while (a != e);
}

static bool push_triangle(
	struct eset *set,
	void *mark,
	float2 *vertices,
	eref e,
	triangle_indices *p)
{
	float2 *v0, *v1, *v2;
	eref e0, e1, e2;

	/* has the triangle on the left already been added? */
	if (bits_get(mark, e >> 1)) { return false; }

	mark_orbit(mark, set, e, lnext);
	e0 = e;
	e1 = lnext(set, e0);
	if (e0 == e1) { return false; }
	e2 = lnext(set, e1);
	if (e1 == e2 || lnext(set, e2) != e0) { return false; }

	v0 = *org(set, e0);
	v1 = *org(set, e1);
	v2 = *org(set, e2);
	if (!is_ccw(*v0, *v1, *v2)) { return false; }

	(*p)[0] = v0 - vertices;
	(*p)[1] = v1 - vertices;
	(*p)[2] = v2 - vertices;
	return true;
}

static struct triangle_set *make_triangles(
	struct eset *set,
	eref e,
	size_t nedges,
	float2 *vertices,
	size_t nvert)
{
	eref a, b, *stack, *top;
	void *origin, *left;
	size_t ntris, nstack, nbits;
	struct triangle_set *t;
	triangle_indices *p;
	struct memblk blk[2];

	assert(set != NULL);
	assert(nvert >= 2);

	/* is there even a single triangle? */
	if (nedges < nvert) { return NULL; }

	/* by euler's characteristic (ignoring the exterior face) */
	ntris = nedges - nvert + 1;
        if (memblk_init(blk+0, 1, sizeof(*t))) { return NULL; }
        if (memblk_push(blk+1, ntris, sizeof(*t->indices), alignof(*t->indices))) {
                return NULL;
        }
        if (t = malloc(blk[1].extent), !t) { return NULL; }
	t->n = ntris;
        t->indices = memblk_offset(t, blk[1]);

	/* stack to keep one outgoing edge of a vertex and a bit map for
	   visited ones */
	nstack = sizeof(eref) * (nvert - 1);
	nbits = bits_size(eset_max_edge(set)*2);
	top = stack = malloc(nstack + nbits*2);
	if (!top) { return free(t), NULL; }
	origin = memset((char *)stack + nstack, 0, nbits*2);
	left = (char *)stack + nstack + nbits;

	/* push starting edge */
	*top++ = e;
	mark_orbit(origin, set, e, onext);

	/* visit every vertex */
	p = t->indices;
	do {
		a = *--top;
		b = a;
		do {
			/* for each outgoing edge */
			b = onext(set, b);
			if (push_triangle(set, left, vertices, b, p)) { p++; }
			if (!bits_get(origin, sym(b) >> 1)) {
				*top++ = sym(b);
				mark_orbit(origin, set, sym(b), onext);
			}
		} while (b != a);
	} while (top != stack);
	t->n = p - t->indices;
	free(stack);
	return t;
}

/* compare points by axis */
static int axis_cmp(float2 *const *va, float2 *const *vb, int axis)
{
	if (axis[**va] < axis[**vb]) {
		return -1;
	} else if (axis[**va] > axis[**vb]) {
		return 1;
	} else {
		return 0;
	}
}

static int vertex_cmp(void const *a, void const *b)
{
	int res;
	float2 *const *va = a, *const *vb = b;
	res = axis_cmp(va, vb, X);
	return res ? res : axis_cmp(va, vb, Y);
}

static float2 **sorted_vertices(float2 *vertices, size_t nmemb)
{
	float2 **v;
	size_t i;

	v = malloc(nmemb * sizeof *v);
	if (!v) { return NULL; }
	for (i = 0; i < nmemb; i++) { v[i] = vertices + i; }
	qsort(v, nmemb, sizeof *v, vertex_cmp);
	return v;
}

/* Create edge from *(vertices + from) to *(vertices + to), removing any edges
   that gets in the way. It is assumed that edge is part of a triangulation
   which forms a convex hull of the points, such that only internal edges can
   be removed. If the edge to be created is too close to a point, then fail by
   returning the index of the point that is too close. The triangulation has
   likely already been botched at this point. Return negative on memory
   allocation error. Return `to` on success. */
static ptrdiff_t add_constrained_edge(
	struct eset *set,
	ptrdiff_t from,
	ptrdiff_t to,
	float2 *vertices,
	eref *emap)
{
	eref e, start, next, new_edge;
	float2 *source, *target;
	struct line2d l;
	double next_dist, dist;
	ptrdiff_t v;

	next = emap[from];
	source = *org(set, next);
	target = vertices + to;
	if (source == target) { return to; }
	l = make_line2d(*source, *target);

	/* Find edge to vertex left of the new edge. In a counter-clockwise
	   order, the edge to the first vertex on the negative side of the line
	   where the previous one was on the positive side is the closest one
	   to the left. */
	dist = next_dist = -1.0;
	while (dist < 0.0 || next_dist > 0.0) {
		e = next;
		dist = next_dist;
		next = onext(set, e);
		if (*dest(set, next) == target) { return to; }
		next_dist = line2d_dist(l, **dest(set, next));
		/* FIXME: if next_dist is too close to line, fail */
	}
	start = e;

	/* follow the orbit of lnext removing any crossing edges on the way */
	while (next = lnext(set, e), *dest(set, next) != target) {
		dist = line2d_dist(l, **dest(set, next));
		/* FIXME: if dist is too close to line, fail */
		if (dist < 0.0) {
			/* make sure it's not part of emap */
			v = *org(set, next) - vertices;
			if (emap[v] == next) {
				emap[v] = sym(e);
			}
			v = *dest(set, next) - vertices;
			if (emap[v] == sym(next)) {
				emap[v] = oprev(set, sym(next));
			}
			eset_delete(set, next);
		} else {
			e = next;
		}
	}

	/* Should not be possible: We have removed at least one edge to
	   get here. */
	if (eset_alloc(set, 1, &new_edge)) { return -1; }
	eset_connect(set, next, start, new_edge);

	if (triangulate_polygon(set, new_edge) < 0) { return -2; }
	if (triangulate_polygon(set, sym(new_edge)) < 0) { return -3; }

	return to;
}

static eref *make_emap(
	struct eset *set,
	eref edge,
	float2 *vertices,
	size_t vertex_count)
{
	eref first_edge, e, *emap, *stack, *top;
	size_t i;

	/* stack of edges for traversing graph, and use emap to keep track
	   of which vertices have been visited */
	emap = malloc(sizeof(eref) * (2*vertex_count - 1));
	if (!emap) { return NULL; }
	top = stack = emap + vertex_count;
	for (i = 0; i < vertex_count; i++) { emap[i] = -1; }

	/* map vertices to edges by traversing edges depth-first */
	*top++ = edge;
	emap[*org(set, edge) - vertices] = edge;
	do {
		first_edge = *--top;
		e = first_edge;
		do {
			/* for each outgoing edge */
			e = onext(set, e);
			i = *dest(set, e) - vertices;
			if (emap[i] == -1) {
				*top++ = emap[i] = sym(e);
			}
		} while (e != first_edge);
	} while (top != stack);

	return emap;
}

static int add_constraints(
	struct eset *set,
	eref edge,
	float2 *vertices,
	size_t vertex_count,
	unsigned const (*constraints)[2],
	size_t constraint_count)
{
	int res;
	eref *emap;
	size_t i;
	unsigned o, n, d;

	assert(set != NULL);
	assert(constraints != NULL);
	assert(constraint_count > 0);

	/* check all constraints */
	res = 0;
	emap = make_emap(set, edge, vertices, vertex_count);
	if (!emap) { return -1; }
	for (i = 0; i < constraint_count; i++) {
		o = constraints[i][0];
		d = constraints[i][1];
		if (o == d) { continue; }
		if (o >= vertex_count || d >= vertex_count) {
			res = -1;
			goto fail;
		}
		n = add_constrained_edge(set, o, d, vertices, emap);
		if (n != d) {
			res = -1;
			goto fail;
		}
	}
fail:	free(emap);

	return res;
}

struct triangle_set *triangle_set_triangulate(
	float2 *vertices,
	size_t nvert,
	unsigned const (*edges)[2],
	size_t nedge)
{
	eref le, re;
	struct eset set;
	float2 **v;
	struct triangle_set *res;
	int final_edges, err;

	if (nvert < 3) { return NULL; }
	v = sorted_vertices(vertices, nvert);
	if (!v) { return NULL; }
	init_eset(&set);
	final_edges = delauney(&set, v, nvert, &le, &re);
	free(v);
	err = 0;
	res = NULL;
	if (final_edges <= 0) { err = -1; }
	if (err == 0 && edges && nedge > 0) {
		err = add_constraints(&set, le, vertices, nvert, edges, nedge);
	}
	if (err == 0) {
		res = make_triangles(&set, le, final_edges, vertices, nvert);
	}
	term_eset(&set);
	return res;
}

void triangle_set_free(struct triangle_set *set)
{
	free(set);
}
