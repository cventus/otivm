#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <limits.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "list.h"
#include "tree.h"

static struct lxlist as_list(struct lxtree tree)
{
	return (struct lxlist) {
		.ref.tag = lx_list_tag,
		.ref.offset = tree.ref.offset,
		.ref.cell = tree.ref.cell,
	};
}

size_t lx_tree_size(struct lxtree tree)
{
	return lx_is_empty_tree(tree) ? 0 : (size_t)ref_data(tree.ref)->i;
}

struct lxlist lx_tree_entry(struct lxtree tree)
{
	return lx_cdr(as_list(tree));
}

struct lxtree lx_tree_left(struct lxtree tree)
{
	return is_leaf_node(tree)
		? lx_empty_tree()
		: deref_tree(ref_data(backward(backward(tree.ref))));
}

struct lxtree lx_tree_right(struct lxtree tree)
{
	return is_leaf_node(tree)
		? lx_empty_tree()
		: deref_tree(ref_data(backward(tree.ref)));
}

static void destructure(
	struct lxtree tree,
	struct lxtree *left,
	struct lxtree *right,
	struct lxlist *entry)
{
	struct lxlist list;
	union lxcell const *sz;
	assert(!lx_is_empty_tree(tree));
	list = as_list(tree);
	*entry = lx_cdr(list);
	if (is_leaf_node(tree)) {
		*left = *right = lx_empty_tree();
	} else {
		list.ref = backward(list.ref);
		sz = ref_data(list.ref);
		*right = isnilref(sz) ? lx_empty_tree() : deref_tree(sz);
		list.ref = backward(list.ref);
		sz = ref_data(list.ref);
		*left = isnilref(sz) ? lx_empty_tree() : deref_tree(sz);
	}
}

struct lxlist lx_tree_nth(struct lxtree tree, lxint n)
{
	struct lxtree t, l, r;
	struct lxlist e;
	size_t sz;

	if (lx_is_empty_tree(tree)) {
		return lx_empty_list();
	}

	t = tree;
	do {
		destructure(t, &l, &r, &e);
		sz = lx_tree_size(l);
		if ((size_t)n < sz) {
			t = l;
		} else if ((size_t)n > sz) {
			t = r;
			n -= sz + 1;
		} else {
			break;
		}
	} while (true);
	return e;
}

struct lxlist lx_tree_assoc(union lxvalue key, struct lxtree tree)
{
	int cmp;
	struct lxtree t, l, r;
	struct lxlist e;

	t = tree;
	while (!lx_is_empty_tree(t)) {
		destructure(t, &l, &r, &e);
		cmp = lx_compare(key, lx_car(e));
		if (cmp < 0) t = l;
		else if (cmp > 0) t = r;
		else return e;
	}
	return lx_empty_list();
}
