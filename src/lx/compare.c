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

#define TWO_TAG_MASK (TAG_MASK | (TAG_MASK << TAG_BIT))
#define TYPEPAIR(a, b) ((((unsigned)a << TAG_BIT) | (unsigned)b) & TWO_TAG_MASK)
#define SAMETYPE(x) TYPEPAIR(x, x)

static int icmp(lxint a, lxint b)
{
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

static int fcmp(lxfloat a, lxfloat b)
{
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

static int listcmp(struct lxlist a, struct lxlist b)
{
	int cmp;
	struct lxlist p, q;

	p = a;
	q = b;
	if (list_eq(p, q)) { return 0; }
	if (!p.value.s) { return -1; }
	if (!q.value.s) { return 1; }
	do {
		cmp = lx_compare(lx_car(p), lx_car(q));
		if (cmp != 0) return cmp;
		p = lx_cdr(p);
		q = lx_cdr(q);
	} while (p.value.s && q.value.s);
	if (!p.value.s && !q.value.s) {
		return 0;
	} else if (!p.value.s) {
		return -1;
	} else {
	 	return 1;
	}
}

static int treecmp(struct lxtree a, struct lxtree b)
{
	int cmp;
	struct lxtree p, q;
	size_t i, asz, bsz, sz;

	p = a;
	q = b;

	if (tree_eq(p, q)) { return 0; }
	if (!p.value.s) { return -1; }
	if (!q.value.s) { return 1; }

	asz = lx_tree_size(a);
	bsz = lx_tree_size(b);
	sz = asz < bsz ? asz : bsz;

	/* tree_nth = O(logn) -> O(nlogn) */
	for (i = 0; i < sz; i++) {
		cmp = listcmp(lx_tree_nth(p, i), lx_tree_nth(q, i));
		if (cmp != 0) { return cmp; }
	}
	return asz == bsz ? 0 : icmp(asz, bsz);
}

int lx_compare(struct lxvalue a, struct lxvalue b)
{
	switch (TYPEPAIR(a.tag, b.tag)) {
	/* bool */
	case SAMETYPE(lx_bool_tag):
		return icmp(a.b, b.b);

	/* int */
	case SAMETYPE(lx_int_tag):
		return icmp(a.i, b.i);
	case TYPEPAIR(lx_int_tag, lx_float_tag):
		return fcmp(a.i, b.f);

	/* float */
	case SAMETYPE(lx_float_tag):
		return fcmp(a.f, b.f);
	case TYPEPAIR(lx_float_tag, lx_int_tag):
		return fcmp(a.f, b.i);

	/* string */
	case SAMETYPE(lx_string_tag):
		return a.s == b.s ? 0 : strcmp(a.s, b.s);

	/* list */
	case SAMETYPE(lx_list_tag):
		return listcmp(lx_list(a), lx_list(b));

	/* tree */
	case SAMETYPE(lx_tree_tag):
		return treecmp(lx_tree(a), lx_tree(b));

	/* type precedence: bool < number < string < list < tree */
	case TYPEPAIR(lx_bool_tag, lx_int_tag):
	case TYPEPAIR(lx_bool_tag, lx_float_tag):
	case TYPEPAIR(lx_bool_tag, lx_string_tag):
	case TYPEPAIR(lx_bool_tag, lx_list_tag):
	case TYPEPAIR(lx_bool_tag, lx_tree_tag):

	case TYPEPAIR(lx_int_tag, lx_string_tag):
	case TYPEPAIR(lx_int_tag, lx_list_tag):
	case TYPEPAIR(lx_int_tag, lx_tree_tag):

	case TYPEPAIR(lx_float_tag, lx_string_tag):
	case TYPEPAIR(lx_float_tag, lx_list_tag):
	case TYPEPAIR(lx_float_tag, lx_tree_tag):

	case TYPEPAIR(lx_string_tag, lx_list_tag):
	case TYPEPAIR(lx_string_tag, lx_tree_tag):

	case TYPEPAIR(lx_list_tag, lx_tree_tag):
		return -1;

	/* type precedence: tree > list > string > number > bool */
	case TYPEPAIR(lx_tree_tag, lx_list_tag):
	case TYPEPAIR(lx_tree_tag, lx_string_tag):
	case TYPEPAIR(lx_tree_tag, lx_float_tag):
	case TYPEPAIR(lx_tree_tag, lx_int_tag):
	case TYPEPAIR(lx_tree_tag, lx_bool_tag):

	case TYPEPAIR(lx_list_tag, lx_bool_tag):
	case TYPEPAIR(lx_list_tag, lx_int_tag):
	case TYPEPAIR(lx_list_tag, lx_float_tag):
	case TYPEPAIR(lx_list_tag, lx_string_tag):

	case TYPEPAIR(lx_string_tag, lx_bool_tag):
	case TYPEPAIR(lx_string_tag, lx_int_tag):
	case TYPEPAIR(lx_string_tag, lx_float_tag):

	case TYPEPAIR(lx_float_tag, lx_bool_tag):

	case TYPEPAIR(lx_int_tag, lx_bool_tag):
		return 1;

	default:
		abort();
		return 0;
	}
}
