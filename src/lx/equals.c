#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>

#include "common.h"
#include "lx.h"
#include "ref.h"
#include "memory.h"
#include "list.h"
#include "tree.h"

static bool list_equals(struct lxlist a, struct lxlist b)
{
	struct lxlist p, q;

	p = a;
	q = b;
	if (list_eq(p, q)) { return true; }
	if (!p.value.s || !q.value.s) { return false; }
	do {
		if (!lx_equals(lx_car(p), lx_car(q))) {
			return false;
		}
		p = lx_cdr(p);
		q = lx_cdr(q);
	} while (p.value.s && q.value.s);
	return list_eq(p, q);
}

static bool map_equals(struct lxmap a, struct lxmap b)
{
	struct lxmap p, q;
	size_t i, sz;

	p = a;
	q = b;

	if (map_eq(p, q)) { return true; }
	if (!p.value.s || !q.value.s) { return false; }

	sz = lx_map_size(a);
	if (sz != lx_map_size(b)) { return false; }

	/* map_nth = O(logn) -> O(nlogn) */
	for (i = 0; i < sz; i++) {
		if (!list_equals(lx_map_nth(p, i), lx_map_nth(q, i))) {
			return false;
		}
	}
	return true;
}

bool lx_equals(struct lxvalue a, struct lxvalue b)
{
	switch (a.tag) {
	default: abort();
	case lx_nil_tag:
		return b.tag == lx_nil_tag;
	case lx_list_tag:
		if (b.tag != lx_list_tag) { return false; }
		return list_equals(lx_list(a), lx_list(b));
	case lx_map_tag:
		if (b.tag != lx_map_tag) { return false; }
		return map_equals(lx_map(a), lx_map(b));
	case lx_string_tag:
		if (b.tag != lx_string_tag) { return false; }
		return a.s == b.s || strcmp(a.s, b.s) == 0;
	case lx_bool_tag:
		if (b.tag != lx_bool_tag) { return false; }
		return a.b == b.b;
	case lx_int_tag:
		switch (b.tag) {
		case lx_int_tag: return a.i == b.i;
		case lx_float_tag: return (lxfloat)a.i == b.f;
		default: return false;
		}
	case lx_float_tag:
		switch (b.tag) {
		case lx_int_tag: return a.f == (lxfloat)b.i;
		case lx_float_tag: return a.f == b.f;
		default: return false;
		}
	}
}
