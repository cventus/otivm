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
#include "memory.h"
#include "ref.h"
#include "list.h"
#include "tree.h"

static bool list_equals(struct lxlist a, struct lxlist b)
{
	struct lxlist p, q;

	p = a;
	q = b;
	if (list_eq(p, q)) { return true; }
	if (!p.ref.cell || !q.ref.cell) { return false; }
	do {
		if (!lx_equals(lx_car(p), lx_car(q))) {
			return false;
		}
		p = lx_cdr(p);
		q = lx_cdr(q);
	} while (p.ref.cell && q.ref.cell);
	return list_eq(p, q);
}

static bool tree_equals(struct lxtree a, struct lxtree b)
{
	struct lxtree p, q;
	size_t i, sz;

	p = a;
	q = b;

	if (tree_eq(p, q)) { return true; }
	if (!p.ref.cell || !q.ref.cell) { return false; }

	sz = lx_tree_size(a);
	if (sz != lx_tree_size(b)) { return false; }

	/* tree_nth = O(logn) -> O(nlogn) */
	for (i = 0; i < sz; i++) {
		if (!list_equals(lx_tree_nth(p, i), lx_tree_nth(q, i))) {
			return false;
		}
	}
	return true;
}

bool lx_equals(union lxvalue a, union lxvalue b)
{
	switch (a.tag) {
	case lx_list_tag:
		if (b.tag != lx_list_tag) { return false; }
		return list_equals(a.list, b.list);
	case lx_tree_tag:
		if (b.tag != lx_tree_tag) { return false; }
		return tree_equals(a.tree, b.tree);
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
	default: return false;
	}
}
