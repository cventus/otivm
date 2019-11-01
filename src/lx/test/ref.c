#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ok/ok.h"

#include "lx32x4.h"

union lxcell cells[25];
struct lxvalue a, b, c, d, e, f, g, h;
struct lxvalue i, j, k, l, m, n, o, p;
struct lxvalue q, r, s, t;

void before_each_test(void)
{
	a.s = b.s = c.s = d.s = (char *)(cells + 0);
	e.s = f.s = g.s = h.s = (char *)(cells + 5);
	i.s = j.s = k.s = l.s = (char *)(cells + 10);
	m.s = n.s = o.s = p.s = (char *)(cells + 15);
	q.s = r.s = s.s = t.s = (char *)(cells + 20);

	a.offset = e.offset = i.offset = m.offset = q.offset = 0;
	b.offset = f.offset = j.offset = n.offset = r.offset = 1;
	c.offset = g.offset = k.offset = o.offset = s.offset = 2;
	d.offset = h.offset = l.offset = p.offset = t.offset = 3;
}

int test_diff(void)
{
	assert(ref_diff(a, j) == -9);
	assert(ref_diff(b, j) == -8);
	assert(ref_diff(c, j) == -7);
	assert(ref_diff(d, j) == -6);
	assert(ref_diff(e, j) == -5);
	assert(ref_diff(f, j) == -4);
	assert(ref_diff(g, j) == -3);
	assert(ref_diff(h, j) == -2);
	assert(ref_diff(i, j) == -1);
	assert(ref_diff(j, j) ==  0);
	assert(ref_diff(k, j) ==  1);
	assert(ref_diff(l, j) ==  2);
	assert(ref_diff(m, j) ==  3);
	assert(ref_diff(n, j) ==  4);
	assert(ref_diff(o, j) ==  5);
	assert(ref_diff(p, j) ==  6);
	assert(ref_diff(q, j) ==  7);
	assert(ref_diff(r, j) ==  8);
	assert(ref_diff(s, j) ==  9);
	assert(ref_diff(t, j) == 10);
	return 0;
}

int test_advance(void)
{
	assert(ref_eq(ref_advance(a,  9), j));
	assert(ref_eq(ref_advance(b,  8), j));
	assert(ref_eq(ref_advance(c,  7), j));
	assert(ref_eq(ref_advance(d,  6), j));
	assert(ref_eq(ref_advance(e,  5), j));
	assert(ref_eq(ref_advance(f,  4), j));
	assert(ref_eq(ref_advance(g,  3), j));
	assert(ref_eq(ref_advance(h,  2), j));
	assert(ref_eq(ref_advance(i,  1), j));

	assert(ref_eq(ref_advance(j,  0), j));

	assert(ref_eq(ref_advance(j,  1), k));
	assert(ref_eq(ref_advance(j,  2), l));
	assert(ref_eq(ref_advance(j,  3), m));
	assert(ref_eq(ref_advance(j,  4), n));
	assert(ref_eq(ref_advance(j,  5), o));
	assert(ref_eq(ref_advance(j,  6), p));
	assert(ref_eq(ref_advance(j,  7), q));
	assert(ref_eq(ref_advance(j,  8), r));
	assert(ref_eq(ref_advance(j,  9), s));
	assert(ref_eq(ref_advance(j, 10), t));
	return 0;
}
