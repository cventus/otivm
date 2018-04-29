#include <stddef.h>
#include <stdlib.h>
#include <stdalign.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <stdio.h>

#include <base/gbuf.h>
#include <base/wbuf.h>
#include <base/mem.h>
#include <base/mempool.h>
#include <adt/ilist.h>

#include "include/shape.h"
#include "include/bezier2.h"
#include "geometry.h"

#define MAX_SPLIT 10

/* Convex hull of a quadratic Bézier spline */
struct hull
{
	struct ilist list, splits;
	struct spline_segment s;
	float area;
	struct lseg2d edges[3];
	size_t depth;
};

struct intersection {
	struct hull *t, *u;
};

static struct hull *next(struct hull *t)
{
	return container_of(t->list.next, struct hull, list);
}

static struct hull *split_next(struct hull *t)
{
	return container_of(t->splits.next, struct hull, splits);
}

static struct hull *make_hull(
	struct mempool *pool,
	struct spline_segment const *s,
	size_t depth)
{
	struct hull *t;

	t = mempool_alloc(pool);
	clist_init(&t->list);
	clist_init(&t->splits);
	t->depth = depth;
	if (s) { (void)memcpy(&t->s, s, sizeof *s); }
	return t;
}

static float area(struct hull *t)
{
	float const *a, *b, *c;

	a = t->s.end;
	b = t->s.mid;
	c = next(t)->s.end;
	return 0.5*fabsf((a[0]-c[0])*(b[1]-a[1]) - (a[0]-b[0])*(c[1]-b[1]));
}

static void update_hull(struct hull *t)
{
	struct hull *u;

	t->area = area(t);
	u = next(t);

	if (is_ccw(t->s.end, t->s.mid, u->s.end)) {
		t->edges[0] = make_lseg2d(u->s.end, t->s.mid);
		t->edges[1] = make_lseg2d(t->s.mid, t->s.end);
		t->edges[2] = make_lseg2d(t->s.end, u->s.end);
	} else {
		t->edges[0] = make_lseg2d(t->s.end, t->s.mid);
		t->edges[1] = make_lseg2d(t->s.mid, u->s.end);
		t->edges[2] = make_lseg2d(u->s.end, t->s.end);
	}
}

void spline_free_shape(struct spline_shape *shape)
{
	free(shape);
}

static bool split(struct mempool *pool, struct hull *t)
{
	enum { d = 3 };

	struct hull *u;
	float p[3*d], q[3*d];

	u = make_hull(pool, NULL, ++t->depth);
	if (!u) { return false; }

	(void)memcpy(p + 0*d, t->s.end, sizeof t->s.end);
	(void)memcpy(p + 1*d, t->s.mid, sizeof t->s.mid);
	(void)memcpy(p + 2*d, next(t)->s.end, sizeof next(t)->s.end);

	p[0*d + 2] = 1.0f;
	p[1*d + 2] = t->s.weight;
	p[2*d + 2] = 1.0f;

	bezier2_split(q, d, p, 0.5);
	t->s.weight = rbezier2_norm_w1(p[0*d + 2], q[0*d + 2], q[1*d + 2]);
	u->s.weight = rbezier2_norm_w1(q[1*d + 2], q[2*d + 2], p[2*d + 2]);

	(void)memcpy(t->s.mid, q + 0*d, sizeof t->s.mid);
	(void)memcpy(u->s.end, q + 1*d, sizeof t->s.end);
	(void)memcpy(u->s.mid, q + 2*d, sizeof t->s.mid);

	clist_insert_next(&t->list, &u->list);
	clist_insert_next(&t->splits, &u->splits);

	update_hull(t);
	update_hull(u);
	
	return true;
}

static int make_linked_spline(
	struct mempool *hullpool,
	struct hull **outlines,
	struct spline_shape const *shape)
{
	size_t i, j;
	struct spline_outline const *outline;
	struct hull *t, *u;

	for (i = 0; i < shape->n; i++) {
		outline = shape->outlines + i;
		if (outline->n == 0) {
			outlines[i] = NULL;
			continue;
		}
		t = make_hull(hullpool, outline->segments, 0);
		outlines[i] = t;
		if (!t) { return -1; }
		for (j = 1; j < outline->n; j++) {
			u = make_hull(hullpool, outline->segments + j, 0);
			if (!u) { return -1; }
			clist_insert_prev(&t->list, &u->list);
		}
	}
	for (i = 0; i < shape->n; i++) {
		t = outlines[i];
		if (!t) continue;
		do {
			update_hull(t);
			t = next(t);
		} while (outlines[i] != t);
	}
	return 0;
}

static bool edges_contain(struct lseg2d const seg[3], float const p[2])
{
	return line2d_dist(seg[0].l, p) > 0.0 &&
		line2d_dist(seg[1].l, p) > 0.0 &&
		line2d_dist(seg[2].l, p) > 0.0;
}

static bool intersect(struct hull *t, struct hull *u)
{
	int i, j;
	float p[2];
	struct lseg2d const *a, *b;

	a = t->edges;
	b = u->edges;

	/* check if edges p0->p1 or p1->p2 intersect */
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 3; j++) {
			if (lseg2d_intersect(p, a[i], b[j])) {
				return true;
			}
		}
	}

	/* check if p0->p2 intersect with q0->q1 or q1->q2 */
	for (i = 0; i < 2; i++) {
		if (lseg2d_intersect(p, a[2], b[i])) {
			return true;
		}
	}

	if (edges_contain(a, u->s.mid)) { return true; }
	if (edges_contain(b, t->s.mid)) { return true; }

	return false;
}

static int intersect_with(
	struct wbuf *buf,
	struct hull *target,
	struct hull **outlines,
	size_t n)
{
	size_t i;
	struct hull *t;
	struct intersection *isec;

	for (i = 0; i < n; i++) {
		t = outlines[i];
		if (!t) continue;
		do {
			if (intersect(t, target)) {
				isec = wbuf_alloc(buf, sizeof *isec);
				if (!isec) { return -1; }
				isec->t = target;
				isec->u = t;
			}
			t = next(t);
		} while (t != outlines[i]);
	}
	return 0;
}

static int find_intersections(
	struct wbuf *buf,
	struct hull **outlines,
	size_t n)
{
	size_t i;
	struct hull *t;

	for (i = 0; i < n; i++) {
		t = outlines[i];
		if (!t) { continue; }
		do {
			if (intersect_with(buf, t, outlines, i + 1)) {
				return -1;
			}
			t = next(t);
		} while (t != outlines[i]);
	}
	return 0;
}

static struct hull *select_which_to_split(struct hull *t, struct hull *u)
{
	if (t->depth < MAX_SPLIT) {
		return t->area > u->area || u->depth >= MAX_SPLIT ? t : u;
	} else {
		return u;
	}
}

static int subdivide(struct wbuf *stack, struct mempool *hullpool)
{
	struct intersection isec;
	struct hull *t, *u, *v;
	bool can_subdivide;

	while (wbuf_size(stack)) {
		wbuf_pop(stack, &isec, sizeof isec);

restart:	t = isec.t;
		u = isec.u;

		do {
			do {
				can_subdivide = t->depth < MAX_SPLIT;
				can_subdivide |= u->depth < MAX_SPLIT;
				if (can_subdivide && intersect(t, u)) {
					v = select_which_to_split(t, u);
					if (!split(hullpool, v)) { return -1; }
					goto restart;
				}
				u = split_next(u);
			} while (u != isec.u);
			t = split_next(t);
		} while (t != isec.t);
	}

	return 0;
}

static int block_layout(
	struct memblk *blk,
	struct spline_shape const *shape,
	size_t *lengths)
{
	typedef struct spline_outline o;
	typedef struct spline_segment s;

	size_t i, n, m;

	n = shape->n;
	m = 0;
	for (i = 0; i < n; i++) {
		m += lengths[i];
	}

	if (memblk_init(blk + 0, 1, sizeof *shape)) { return -1; }
	if (memblk_push(blk + 1, n, sizeof(o), alignof(o))) { return -1; }
	if (memblk_push(blk + 2, m, sizeof(s), alignof(s))) { return -1; }

	return 0;
}

static struct spline_shape *make_shape_block(
	struct spline_shape const *shape,
	struct hull **outlines,
	size_t *lengths)
{
	struct spline_shape *result;
	struct spline_outline *o;
	struct spline_segment *s;
	struct memblk blk[3];
	size_t i, j;
	struct hull *t;

	if (block_layout(blk, shape, lengths)) { return NULL; }
	result = malloc(blk[2].extent);
	if (!result) { return NULL; }
	o = memblk_offset(result, blk[1]);
	s = memblk_offset(result, blk[2]);

	result->n = shape->n;
	result->outlines = o;

	for (i = 0; i < shape->n; i++) {
		o->n = lengths[i];
		o->segments = s;
		t = outlines[i];
		for (j = 0; j < o->n; j++) {
			(void)memcpy(s, &t->s, sizeof *s);
			t = next(t);
			s++;
		}
		o++;
	}

	return result;
}

struct spline_shape *spline_simplify_shape(struct spline_shape const *shape)
{
	struct spline_shape *result;
	struct hull **outlines, *outline_buf[16];
	struct wbuf stack;
	size_t i, sz, *lengths, lengths_buf[length_of(outline_buf)];
	struct mempool hullpool;

	if (shape->n < length_of(outline_buf)) {
		outlines = outline_buf;
		lengths = lengths_buf;
	} else {
		sz = align_to(shape->n * sizeof*outlines, alignof(size_t));
		outlines = malloc(sz + sizeof(size_t) * shape->n);
		lengths = (size_t *)((char *)outlines + sz);
	}

	mempool_init(&hullpool, 100, sizeof (struct hull));
	wbuf_init(&stack);
	result = NULL;

	if (make_linked_spline(&hullpool, outlines, shape)) { goto fail; }
	if (find_intersections(&stack, outlines, shape->n)) { goto fail; }
	if (subdivide(&stack, &hullpool)) { goto fail; }

	for (i = 0; i < shape->n; i++) {
		lengths[i] = clist_length(&outlines[i]->list);
	}
	result = make_shape_block(shape, outlines, lengths);

fail:	if (outlines != outline_buf) { free(outlines); }
	mempool_term(&hullpool);
	wbuf_term(&stack);

	return result;
}
