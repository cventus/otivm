#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <base/wbuf.h>
#include <base/mem.h>

#include "include/triangulate.h"

#define X 0
#define Y 1
#define XY 2

typedef unsigned int eref;

#define EREF(e0, r) (((e0) & ~0x3) | ((r) & 0x3))
#define MAX_EDGES (~(eref)0 >> 2)
#define EPSILON 1e-8 /* in determinants (sums/differnces close to zero) */

struct vertex
{
	float xy[XY];
	size_t index;
};

struct edge
{
	void const *data;
	eref next;
};

struct eset
{
	struct wbuf edges;
	size_t count;
};

static eref rot(eref e) { return EREF(e, e + 1); }
static eref sym(eref e) { return EREF(e, e + 2); }
static eref invrot(eref e) { return EREF(e, e + 3); }

static eref onext(struct eset *set, eref e)
{
	assert(e < wbuf_nmemb(&set->edges, sizeof(struct edge)));
	return ((struct edge *)set->edges.begin)[e].next;
}

static eref oprev(struct eset *set, eref e)
{
	return rot(onext(set, rot(e)));
}

static eref lnext(struct eset *set, eref e)
{
	return rot(onext(set, invrot(e)));
}

static eref rnext(struct eset *set, eref e)
{
	return invrot(onext(set, rot(e)));
}

static eref rprev(struct eset *set, eref e)
{
	return onext(set, sym(e));
}

static void init_eset(struct eset *set)
{
	wbuf_init(&set->edges);
	set->count = 0;
}

static void term_eset(struct eset *set)
{
	wbuf_term(&set->edges);
}

static void const **org(struct eset *set, eref e)
{
	return &((struct edge *)set->edges.begin)[e].data;
}

static float const *org_xy(struct eset const *set, eref e)
{
	struct vertex const *vertex = ((struct edge *)set->edges.begin)[e].data;
	return vertex->xy;
}

static size_t org_idx(struct eset const *set, eref e)
{
	struct vertex const *vertex = ((struct edge *)set->edges.begin)[e].data;
	return vertex->index;
}

static void const **right(struct eset *set, eref e)
{
	return org(set, rot(e));
}

static void const **dest(struct eset *set, eref e)
{
	return org(set, sym(e));
}

static float const *dest_xy(struct eset const *set, eref e)
{
	return org_xy(set, sym(e));
}

static size_t dest_idx(struct eset const *set, eref e)
{
	return org_idx(set, sym(e));
}

/* allocate *n* empty subdivisions (edges) and store references (eref) in the
   *n* following arguments */
static int make_edges(struct eset *set, size_t n, ...)
{
	struct edge *p;
	eref e, *q;
	size_t i;
	va_list ap;

	p = wbuf_alloc(&set->edges, n * sizeof (struct edge[4]));
	set->count += n;
	if (!p) { return -1; }
	va_start(ap, n);
	for (i = 0; i < n; i++) {
		q = va_arg(ap, eref *);
		*q = e = p - (struct edge *)set->edges.begin;
		p[0].data = p[1].data = p[2].data = p[3].data = 0;
		p[0].next = e;
		p[1].next = e + 3;
		p[2].next = e + 2;
		p[3].next = e + 1;
		p += 4;
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

/* connect a and b with new empty edge c */
static void connect(struct eset *set, eref a, eref b, eref c)
{
	*org(set, c) = *dest(set, a);
	*dest(set, c) = *org(set, b);
	splice(set, c, lnext(set, a));
	splice(set, sym(c), b);
}

static void delete_edge(struct eset *set, eref e)
{
	/* TODO: free list */
	set->count--;
	splice(set, e, oprev(set, e));
	splice(set, sym(e), oprev(set, sym(e)));
}

static bool is_ccw(float const a[XY], float const b[XY], float const c[XY])
{
	/* counterclockwise 2D points if

		| X[a] Y[a] 1 |
		| X[b] Y[b] 1 | > 0
		| X[c] Y[c] 1 |
	*/
	double p = X[a]*(double)Y[b] + X[b]*(double)Y[c] + X[c]*(double)Y[a];
	double n = X[c]*(double)Y[b] + X[b]*(double)Y[a] + X[a]*(double)Y[c];
	return p - n > EPSILON;
}

static bool in_circle(
	float const a[XY],
	float const b[XY],
	float const c[XY],
	float const d[XY])
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

static bool is_right_of(struct eset *set, float const p[XY], eref e)
{
	return is_ccw(p, dest_xy(set, e), org_xy(set, e));
}

static bool is_left_of(struct eset *set, float const p[XY], eref e)
{
	return is_ccw(p, org_xy(set, e), dest_xy(set, e));
}

static bool is_valid(struct eset *set, eref e, eref basel)
{
	return is_right_of(set, dest_xy(set, e), basel);
}

static eref select_cand(
	struct eset *set,
	eref basel,
	eref init,
	eref next(struct eset *, eref))
{
	eref cand, tmp;

	cand = next(set, init);
	if (is_valid(set, cand, basel)) {
		while (in_circle(dest_xy(set, basel),
		                 org_xy(set, basel),
		                 dest_xy(set, cand),
		                 dest_xy(set, next(set, cand)))) {
			tmp = next(set, cand);
			delete_edge(set, cand);
			cand = tmp;
		}
	}
	return cand;
}

/* implementation of Guibas & Stolfi (1985) */
static int delauney(
	struct eset *set,
	struct vertex const *verts,
	size_t nmemb,
	eref *le,
	eref *re)
{
	assert(nmemb >= 2);
	if (nmemb == 2) {
		/* a single edge connecting the points */
		eref a;
		if (make_edges(set, 1, &a)) { return -1; }
		*org(set, a) = verts + 0;
		*dest(set, a) = verts + 1;
		*le = a;
		*re = sym(a);
		return 0;
	} else if (nmemb == 3) {
		/* A = 0 -> 1, and B = 1 -> 2 */
		eref a, b, c;
		if (make_edges(set, 2, &a, &b)) { return -1; }
		splice(set, sym(a), b);
		*org(set, a) = verts + 0;
		*dest(set, a) = *org(set, b) = verts + 1;
		*dest(set, b) = verts + 2;
		if (is_ccw(verts[0].xy, verts[2].xy, verts[1].xy)) {
			if (make_edges(set, 1, &c)) { return -1; }
			connect(set, b, a, c);
			*le = sym(c);
			*re = c;
			return 0;
		} else {
			if (is_ccw(verts[0].xy, verts[1].xy, verts[2].xy)) {
				if (make_edges(set, 1, &c)) { return -1; }
				connect(set, b, a, c);
			} /* else, co-linear */
			*le = a;
			*re = sym(b);
			return 0;
		}
	} else {
		size_t ln, rn;
		eref ldo, ldi, rdi, rdo, basel, lcand, rcand, tmp;
		bool lvalid, rvalid;

		ln = nmemb / 2;
		rn = nmemb - ln;
		if (delauney(set, verts, ln, &ldo, &ldi)) { return -1; }
		if (delauney(set, verts + ln, rn, &rdi, &rdo)) { return -1; }

		/* find lower common tangent of L and R */
		while (1) {
			if (is_left_of(set, org_xy(set, rdi), ldi)) {
				ldi = lnext(set, ldi);
			} else if (is_right_of(set, org_xy(set, ldi), rdi)) {
				rdi = rprev(set, rdi);
			} else break;
		}

		/* connect left and right subdivisions */
		if (make_edges(set, 1, &basel)) { return -1; }
		connect(set, sym(rdi), ldi, basel);
		if (*org(set, ldi) == *org(set, ldo)) { ldo = sym(basel); }
		if (*org(set, rdi) == *org(set, rdo)) { rdo = basel; }

		/* proceed by merging upwards */
		do {
			lcand = select_cand(set, basel, sym(basel), onext);
			rcand = select_cand(set, basel, basel, oprev);

			/* are we done? */
			lvalid = is_valid(set, lcand, basel);
			rvalid = is_valid(set, rcand, basel);
			if (!lvalid && !rvalid) { break; }

			/* else add cross edge */
			if (make_edges(set, 1, &tmp)) { return -1; }
			if (!lvalid ||
			    (rvalid && in_circle(dest_xy(set, lcand),
			                         org_xy(set, lcand),
			                         org_xy(set, rcand),
			                         dest_xy(set, rcand)))) {
				connect(set, rcand, sym(basel), tmp);
			} else {
				connect(set, sym(basel), sym(lcand), tmp);
			}
			/* continue building from it */
			basel = tmp;
		} while (1);
		*le = ldo;
		*re = rdo;
		return 0;
	}
}

#define MSB (1 << (CHAR_BIT - 1))
static size_t bits_size(size_t n) { return (n + CHAR_BIT - 1) / CHAR_BIT; }

static void bits_set(void *bits, size_t i)
{
	((unsigned char *)bits)[i / CHAR_BIT] |= (MSB >> (i % CHAR_BIT));
}

static bool bits_get(void *bits, size_t i)
{
	return ((unsigned char *)bits)[i / CHAR_BIT] & (MSB >> (i % CHAR_BIT));
}

static bool push_triangle(struct eset *set, eref e, struct triangle *p)
{
	struct vertex const *v0, *v1, *v2;
	eref e0, e1, e2;

	if (*right(set, e)) { return false; }

	e0 = e;
	e1 = rnext(set, e0);
	e2 = rnext(set, e1);
	if (rnext(set, e2) != e0) { return false; }

	v0 = *org(set, e0);
	v1 = *org(set, e1);
	v2 = *org(set, e2);

	if (is_ccw(v0->xy, v1->xy, v2->xy)) {
		p->a = v0->index;
		p->b = v1->index;
		p->c = v2->index;
		*right(set, e0) = *right(set, e1) = *right(set, e2) = p;
		return true;
	} else {
		return false;
	}
}

static struct triangulation *make_triangles(
	struct eset *set,
	eref e,
	size_t nvert)
{
	eref a, b, *stack, *top;
	void *mark;
	size_t ntris, nstack, nbits;
	struct triangulation *t;
	struct triangle *p;

	assert(set != NULL);
	assert(nvert >= 2);

	if (set->count < nvert) {
		/* not even a single triangle */
		return NULL; 
	}

	/* by euler's characteristic (ignore the exterior face) */
	ntris = set->count - nvert + 1;
	t = malloc(sizeof(*t) + ntris*sizeof t->triangles[0]);
	if (!t) { return NULL; }
	t->n = ntris;

	/* stack to keep one outgoing edge of a vertex and a bit map for
	   visited ones */
	nstack = sizeof(eref) * (nvert - 1);
	nbits = bits_size(nvert);
	top = stack = malloc(nstack + nbits);
	if (!top) { return free(t), NULL; }
	mark = memset((char *)stack + nstack, 0, nbits);

	/* push starting edge */
	*top++ = e;
	bits_set(mark, org_idx(set, e));

	/* visit every vertex */
	p = t->triangles;
	do {
		a = *--top;
		b = a;
		do {
			/* for each outgoing edge */
			b = onext(set, b);
			if (push_triangle(set, b, p)) { p++; }
			if (!bits_get(mark, dest_idx(set, b))) {
				*top++ = sym(b);
				bits_set(mark, dest_idx(set, b));
			}
		} while (b != a);
	} while (top != stack);
	free(stack);
	return t;
}

/* compare points by axis */
static int axis_cmp(struct vertex const *va, struct vertex const *vb, int axis)
{
	if (axis[va->xy] < axis[vb->xy]) {
		return -1;
	} else if (axis[va->xy] > axis[vb->xy]) {
		return 1;
	} else {
		return 0;
	}
}

static int pt_cmp(void const *a, void const *b)
{
	int res;
	struct vertex const *va = a, *vb = b;
	res = axis_cmp(va, vb, X);
	return res ? res : axis_cmp(va, vb, Y);
}

struct triangulation *triangulate(float const **vertices, size_t nmemb)
{
	eref le, re;
	struct eset set;
	struct vertex *v;
	struct triangulation *res;
	size_t i;

	if (nmemb < 3) { return NULL; }
	v = calloc(nmemb, sizeof(struct vertex));
	if (!v) { return NULL; }
	for (i = 0; i < nmemb; i++) {
		v[i].index = i;
		X[v[i].xy] = X[vertices[i]];
		Y[v[i].xy] = Y[vertices[i]];
	}
	qsort(v, nmemb, sizeof *v, pt_cmp);
	// TODO: Handle duplicate coordinates?
	init_eset(&set);
	if (delauney(&set, v, nmemb, &le, &re) == 0) {
		res = make_triangles(&set, le, nmemb);
	} else {
		res = NULL;
	}
	term_eset(&set);
	free(v);
	return res;
}
