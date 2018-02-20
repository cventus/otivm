#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <base/wbuf.h>
#include <base/mem.h>

#include "include/triangulate.h"

#define X 0
#define Y 1
#define XY 2

#define EPSILON 1e-8
#define MSB (1 << (CHAR_BIT - 1))

typedef unsigned triangle_indices[3];
typedef int eref;
typedef float const float2[XY];

struct edge
{
	float2 *data;
	eref next;
};

struct eset
{
	struct wbuf edges;
	eref free;
};

struct line
{
	double a, b, c;
};

static inline eref mkref(eref e0, unsigned r) { return (e0 & ~0x3)|(r & 0x3); }
static inline eref rot(eref e) { return mkref(e, e + 1); }
static inline eref sym(eref e) { return mkref(e, e + 2); }
static inline eref invrot(eref e) { return mkref(e, e + 3); }

static inline eref onext(struct eset *set, eref e)
{
	assert(e >= 0);
	assert((size_t)e < wbuf_nmemb(&set->edges, sizeof(struct edge)));
	return ((struct edge *)set->edges.begin)[e].next;
}

static inline eref oprev(struct eset *set, eref e)
{
	return rot(onext(set, rot(e)));
}

static inline eref lnext(struct eset *set, eref e)
{
	return rot(onext(set, invrot(e)));
}

static inline eref lprev(struct eset *set, eref e)
{
	return sym(onext(set, e));
}

static inline eref rnext(struct eset *set, eref e)
{
	return invrot(onext(set, rot(e)));
}

static inline eref rprev(struct eset *set, eref e)
{
	return onext(set, sym(e));
}

static inline float2 **org(struct eset *set, eref e)
{
	return &((struct edge *)set->edges.begin)[e].data;
}

static inline float2 **dest(struct eset *set, eref e)
{
	return org(set, sym(e));
}

static void init_eset(struct eset *set)
{
	wbuf_init(&set->edges);
	set->free = -1;
}

static eref eset_max_edge(struct eset *set)
{
	return wbuf_nmemb(&set->edges, sizeof (struct edge[4]));
}

static void term_eset(struct eset *set)
{
	wbuf_term(&set->edges);
}

static void init_quad_edge(struct edge *qe, eref e0)
{
	qe[0].next = mkref(e0, 0);
	qe[1].next = mkref(e0, 3);
	qe[2].next = mkref(e0, 2);
	qe[3].next = mkref(e0, 1);
}

/* allocate *n* empty subdivisions (edges) and store references (eref) in the
   *n* following arguments */
static int make_edges(struct eset *set, size_t n, ...)
{
	struct edge *alloc, *p, *edges;
	eref e0;
	size_t i, j, nfree;
	va_list ap;

	/* look in free list first */
	e0 = set->free;
	edges = set->edges.begin;
	nfree = 0;
	while (e0 >= 0 && nfree < n) {
		e0 = edges[e0].next;
		nfree++;
	}
	/* allocate the rest */
	alloc = wbuf_alloc(&set->edges, (n - nfree) * sizeof (struct edge[4]));
	if (!alloc) { return -1; }
	edges = set->edges.begin;

	va_start(ap, n);
	for (i = 0, j = 0; i < n; i++) {
		if (j < nfree) {
			e0 = set->free;
			set->free = edges[e0].next;
			p = edges + e0;
			j++;
		} else {
			p = alloc + 4*(i - j);
			e0 = p - edges;
		}
		*va_arg(ap, eref *) = e0;
		p[0].data = p[1].data = p[2].data = p[3].data = 0;
		init_quad_edge(p, e0);
	}
	va_end(ap);
	return 0;
}

static void splice(struct eset *set, eref a, eref b)
{
	eref alpha, beta, tmp;
	struct edge *edges;

	edges = set->edges.begin;
	alpha = rot(onext(set, a));
	beta = rot(onext(set, b));

	tmp = edges[a].next;
	edges[a].next = edges[b].next;
	edges[b].next = tmp;

	tmp = edges[alpha].next;
	edges[alpha].next = edges[beta].next;
	edges[beta].next = tmp;
}

/* connect the destination of `a` to the origin of `b` with the new edge `c` so
   that left(a) = left(b) = left(c) */
static void connect(struct eset *set, eref a, eref b, eref c)
{
	*org(set, c) = *dest(set, a);
	*dest(set, c) = *org(set, b);
	splice(set, c, lnext(set, a));
	splice(set, sym(c), b);
}

static void delete_edge(struct eset *set, eref e)
{
	splice(set, e, oprev(set, e));
	splice(set, sym(e), oprev(set, sym(e)));

	/* add to free list */
	((struct edge *)set->edges.begin)[e & ~0x3].next = set->free;
	set->free = e & ~0x3;
}

static double line_det(float2 a, float2 b, float2 c)
{
	/* The determinant D of the linear inequality specifies on which side
	   of line `b->c` that point `a` lies

		| X[a] Y[a] 1 |
		| X[b] Y[b] 1 | = D
		| X[c] Y[c] 1 |
	*/
	double p = X[a]*Y[b] + X[b]*Y[c] + X[c]*Y[a];
	double n = X[c]*Y[b] + X[b]*Y[a] + X[a]*Y[c];
	return p - n;
}

static struct line make_line(float2 p0, float2 p1)
{
	double dx, dy, a, b, c, s;

	dy = Y[p1] - Y[p0];
	dx = X[p1] - X[p0];
	s = hypot(dx, dy);
	a = dy / s;
	b = dx / s;
	c = Y[p1]*b - X[p1]*a;

	return (struct line){ a, b, c };
}

/* Signed distance from p to l. If the result is zero, then the point is on the
   line. If it is greater than zero then it is on the "right" side of l, where
   the orientation is determined when the line was created. A negative result
   indicates the point is to the "left". */
static double line_dist(struct line l, float2 p)
{
	return l.a*X[p] - l.b*Y[p] + l.c;
}

static bool in_circle(float2 a, float2 b, float2 c, float2 d)
{
	/* point d within circle specified by (a, b, c), if

		| X[a]-X[d]  Y[a]-Y[d]  ((X[a]-X[d])^2 + (Y[a]-Y[d])^2) |
		| X[b]-X[d]  Y[b]-Y[d]  ((X[b]-X[d])^2 + (Y[b]-Y[d])^2) | > 0
		| X[c]-X[d]  Y[c]-Y[d]  ((X[c]-X[d])^2 + (Y[c]-Y[d])^2) |
	*/
	double dx0 = X[a] - X[d];
	double dx1 = X[b] - X[d];
	double dx2 = X[c] - X[d];

	double dy0 = Y[a] - Y[d];
	double dy1 = Y[b] - Y[d];
	double dy2 = Y[c] - Y[d];

	double d20 = dx0*dx0 + dy0*dy0;
	double d21 = dx1*dx1 + dy1*dy1;
	double d22 = dx2*dx2 + dy2*dy2;

	double p0 = dx0 * dy1 * d22;
	double p1 = dx2 * dy0 * d21;
	double p2 = dx1 * dy2 * d20;

	double n0 = dx2 * dy1 * d20;
	double n1 = dx0 * dy2 * d21;
	double n2 = dx1 * dy0 * d22;

	return p0 - n0 + p1 - n1 + p2 - n2 > EPSILON;
}

/* triangle defined by the positive side of three lines */
static bool in_triangle(struct line const tri[3], float2 p)
{
	return line_dist(tri[0], p) >= -EPSILON &&
		line_dist(tri[1], p) >= -EPSILON &&
		line_dist(tri[2], p) >= -EPSILON;
}

static bool is_ccw(float2 a, float2 b, float2 c)
{
	/* counterclockwise 2D points if on the "left" side of the line going
	   from b to c */
	return line_det(a, b, c) > EPSILON;
}

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
	struct line perim[3];

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
	if (make_edges(set, 1, &e)) { return -2; }
	/* find diagonal or create an edge from dest(a) to org(b) */
	perim[0] = make_line(*p1, *p0);
	perim[1] = make_line(*p2, *p1);
	perim[2] = make_line(*p0, *p2);
	max_dist = -1.0;
	c = b;
	end = lprev(set, a);
	while (c = lnext(set, c), c != end) {
		p = *dest(set, c);
		if (in_triangle(perim, *p)) {
			/* candidate found */
			dist = line_dist(perim[2], *p);
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
		connect(set, d, b, e);
		res = triangulate_polygon(set, e);
		if (res >= 0) {
			count += res;
			res = triangulate_polygon(set, sym(e));
		}
	} else {
		/* add new edge e, forming triangle a->e->b->a */
		connect(set, b, a, e);

		/* triangulate the rest of the polygon */
		res = triangulate_polygon(set, sym(e));
	}
	if (res < 0) {
		delete_edge(set, e);
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
		while (in_circle(**dest(set, basel),
		                 **org(set, basel),
		                 **dest(set, cand),
		                 **dest(set, next(set, cand)))) {
			tmp = next(set, cand);
			delete_edge(set, cand);
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
		if (make_edges(set, 1, &a)) { return -1; }
		*org(set, a) = verts[0];
		*dest(set, a) = verts[1];
		*le = a;
		*re = sym(a);
		return 1;
	} else if (nmemb == 3) {
		/* A = 0 -> 1, and B = 1 -> 2 */
		eref a, b, c;
		if (make_edges(set, 2, &a, &b)) { return -1; }
		splice(set, sym(a), b);
		*org(set, a) = verts[0];
		*dest(set, a) = *org(set, b) = verts[1];
		*dest(set, b) = verts[2];
		if (is_ccw(*verts[0], *verts[2], *verts[1])) {
			if (make_edges(set, 1, &c)) { return -1; }
			connect(set, b, a, c);
			*le = sym(c);
			*re = c;
			return 3;
		} else {
			count = 2;
			if (is_ccw(*verts[0], *verts[1], *verts[2])) {
				if (make_edges(set, 1, &c)) { return -1; }
				count++;
				connect(set, b, a, c);
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
		if (make_edges(set, 1, &basel)) { return -1; }
		connect(set, sym(rdi), ldi, basel);
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
			if (make_edges(set, 1, &tmp)) { return -1; }
			count++;
			if (!lvalid ||
			    (rvalid && in_circle(**dest(set, lcand),
			                         **org(set, lcand),
			                         **org(set, rcand),
			                         **dest(set, rcand)))) {
				connect(set, rcand, sym(basel), tmp);
			} else {
				connect(set, sym(basel), sym(lcand), tmp);
			}
			/* continue building from it */
			basel = tmp;
		} while (1);
		*le = ldo;
		*re = rdo;
		return count;
	}
}

static size_t bits_size(size_t n) { return (n + CHAR_BIT - 1) / CHAR_BIT; }

static void bits_set(void *bits, size_t i)
{
	((unsigned char *)bits)[i / CHAR_BIT] |= (MSB >> (i % CHAR_BIT));
}

static bool bits_get(void *bits, size_t i)
{
	return ((unsigned char *)bits)[i / CHAR_BIT] & (MSB >> (i % CHAR_BIT));
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

	assert(set != NULL);
	assert(nvert >= 2);

	/* is there even a single triangle? */
	if (nedges < nvert) { return NULL; }

	/* by euler's characteristic (ignoring the exterior face) */
	ntris = nedges - nvert + 1;
	t = malloc(triangle_set_size(ntris));
	if (!t) { return NULL; }
	t->n = ntris;

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
	struct line l;
	double next_dist, dist;
	ptrdiff_t v;

	next = emap[from];
	source = *org(set, next);
	target = vertices + to;
	if (source == target) { return to; }
	l = make_line(*source, *target);

	/* Find edge to vertex left of the new edge. In a counter-clockwise
	   order, the edge to the first vertex on the negative side of the line
	   where the previous one was on the positive side is the closest one
	   to the left. */
	dist = 1.0;
	next_dist = 1.0;
	while (dist < 0.0 || next_dist > 0.0) {
		e = next;
		dist = next_dist;
		next = onext(set, e);
		if (*dest(set, next) == target) { return to; }
		next_dist = line_dist(l, **dest(set, next));
		/* FIXME: if next_dist is too close to line, fail */
	}
	start = e;

	/* follow the orbit of lnext removing any crossing edges on the way */
	while (next = lnext(set, e), *dest(set, next) != target) {
		dist = line_dist(l, **dest(set, next));
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
			delete_edge(set, next);
		} else {
			e = next;
		}
	}

	/* Should not be possible: We have removed at least one edge to
	   get here. */
	if (make_edges(set, 1, &new_edge)) { return -1; }
	connect(set, next, start, new_edge);

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
	struct triangle_set const *constr)
{
	int res;
	eref *emap;
	size_t i, j, m;
	unsigned o, n, d;

	assert(set != NULL);
	assert(constr != NULL);

	/* check all constraints */
	res = 0;
	emap = make_emap(set, edge, vertices, vertex_count);
	if (!emap) { return -1; }
	m = length_of(constr->indices[0]);
	for (i = 0; i < constr->n; i++) {
		for (j = 0; j < m; j++) {
			o = constr->indices[i][j];
			d = constr->indices[i][(j + 1) % m];
			if (o == d) { continue; }
			n = add_constrained_edge(set, o, d, vertices, emap);
			if (n != d) {
				res = -1;
				goto fail;
			}
		}
	}
fail:	free(emap);

	return res;
}

struct triangle_set *triangulate(
	float2 *vertices,
	size_t vertex_count,
	struct triangle_set const *constr)
{
	eref le, re;
	struct eset set;
	float2 **v;
	struct triangle_set *res;
	int edges, err;

	if (vertex_count < 3) { return NULL; }
	v = sorted_vertices(vertices, vertex_count);
	if (!v) { return NULL; }
	init_eset(&set);
	edges = delauney(&set, v, vertex_count, &le, &re);
	free(v);
	err = 0;
	res = NULL;
	if (edges > 0) {
		if (constr && constr->n > 0) {
			err = add_constraints(
				&set, le, vertices, vertex_count, constr);
		}
		if (err == 0) {
			res = make_triangles(
				&set, le, edges, vertices, vertex_count);
		}
	}
	term_eset(&set);
	return res;
}
