#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include <base/wbuf.h>

#include "geometry.h"
#include "quadedge.h"

typedef eref quadedge[4];

void init_eset(struct eset *set)
{
	wbuf_init(&set->edges);
	wbuf_init(&set->data);
	set->free = -1;
}

eref eset_max_edge(struct eset *set)
{
	return wbuf_nmemb(&set->edges, sizeof (quadedge));
}

void term_eset(struct eset *set)
{
	wbuf_term(&set->edges);
	wbuf_term(&set->data);
}

void init_quadedge(eref *qe, float2 **data, eref e0)
{
	qe[e0 + 0] = mkref(e0, 0);
	qe[e0 + 1] = mkref(e0, 3);
	qe[e0 + 2] = mkref(e0, 2);
	qe[e0 + 3] = mkref(e0, 1);

	data[e0 >> 1] = 0;
	data[(e0 >> 1) + 1] = 0;
}

/* allocate *n* empty subdivisions (edges) and store references (eref) in the
   *n* following arguments */
int eset_alloc(struct eset *set, size_t n, ...)
{
	eref *alloc, *p, *edges, e0;
	float2 **data;
	size_t i, j, nfree;
	va_list ap;

	/* look in free list first */
	e0 = set->free;
	edges = set->edges.begin;
	nfree = 0;
	while (e0 >= 0 && nfree < n) {
		e0 = edges[e0];
		nfree++;
	}
	/* allocate the rest */
	alloc = wbuf_alloc(&set->edges, (n - nfree) * sizeof (quadedge));
	if (!alloc) { return -1; }
	if (!wbuf_alloc(&set->data, (n - nfree) * sizeof (float2 *[2]))) {
		return -1;
	}
	edges = set->edges.begin;
	data = set->data.begin;

	va_start(ap, n);
	for (i = 0, j = 0; i < n; i++) {
		if (j < nfree) {
			e0 = set->free;
			set->free = edges[e0];
			p = edges + e0;
			j++;
		} else {
			p = alloc + 4*(i - j);
			e0 = p - edges;
		}
		*va_arg(ap, eref *) = e0;
		init_quadedge(edges, data, e0);
	}
	va_end(ap);
	return 0;
}

void eset_splice(struct eset *set, eref a, eref b)
{
	eref alpha, beta, tmp, *edges;

	edges = set->edges.begin;
	alpha = rot(onext(set, a));
	beta = rot(onext(set, b));

	tmp = edges[a];
	edges[a] = edges[b];
	edges[b] = tmp;

	tmp = edges[alpha];
	edges[alpha] = edges[beta];
	edges[beta] = tmp;
}

/* connect the destination of `a` to the origin of `b` with the new edge `c` so
   that left(a) = left(b) = left(c) */
void eset_connect(struct eset *set, eref a, eref b, eref c)
{
	*org(set, c) = *dest(set, a);
	*dest(set, c) = *org(set, b);
	eset_splice(set, c, lnext(set, a));
	eset_splice(set, sym(c), b);
}

void eset_delete(struct eset *set, eref e)
{
	eset_splice(set, e, oprev(set, e));
	eset_splice(set, sym(e), oprev(set, sym(e)));

	/* add to free list */
	((eref *)set->edges.begin)[e & ~0x3] = set->free;
	set->free = e & ~0x3;
}
