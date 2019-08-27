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

static struct lxtree as_tree(struct lxlist list)
{
	return (struct lxtree) {
		.ref.tag = lx_tree_tag,
		.ref.offset = list.ref.offset,
		.ref.cell = list.ref.cell,
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

static bool is_balanced(struct lxtree a, struct lxtree b)
{
	/* No risk of size_t overflow:
	 * - each tree node requires four tagged cells
	 * - for 16 bit
	 *   - 2 segments per tree node
	 *   - less than 2^14/3 tree nodes in a heap
	 *   -> 3 * (size(a) + 1) < 3*(2^14/3 + 1) < 2^16
	 * - for 32 bit
	 *   - 1 segments per tree node
	 *   - less than 2^31/5 tree nodes in a heap
	 *   -> 3 * (size(a) + 1) < 3*(2^31/5 + 1) < 2^32
	 * - for 64 bit
	 *   - two tree nodes per segment
	 *   - less than 2^64/9 tree nodes in a heap
	 *   -> 3 * (size(a) + 1) < 3*(2^64/5 + 1) = 2^64/3 + 3 < 2^64
	 */
	/* delta = 3 */
	return 3 * (lx_tree_size(a) + 1) >= lx_tree_size(b) + 1;
}

static bool is_single(struct lxtree a, struct lxtree b)
{
	/* if 3*(size(x) + 1) doesn't overflow, neither does 2*size(b) */
	/* gamma = 2 */
	return lx_tree_size(a) + 1 < 2 * (lx_tree_size(b) + 1);
}

static struct lxtree make_leaf_node(struct lxmem *mem, struct lxlist entry)
{
	assert(!lx_is_empty_list(entry));
	return as_tree(lx_cons(mem, lx_int(1), entry));
}

static inline lxtag next_length(lxtag len) {
	if (len == 0) {
		return 2;
	} else if (len == MAX_SEGMENT_LENGTH) {
		return len;
	} else {
		return len + 1;
	}
}

static struct lxtree make_node(
	struct lxmem *mem,
	struct lxtree l,
	struct lxtree r,
	struct lxlist e)
{
	struct lxtree size, left, right, data;
	lxint len, entry_len;
	bool is_adjacent;
	union lxvalue car;

	assert(!lx_is_empty_list(e));
	assert(l.tag == lx_tree_tag);
	assert(r.tag == lx_tree_tag);
	assert(e.tag == lx_list_tag);

	if (lx_is_empty_tree(l) && lx_is_empty_tree(r)) {
		return make_leaf_node(mem, e);
	}

	/* allocate node */
	is_adjacent = ref_eq(e.ref, mem->alloc.tag_free);
	if (lx_reserve_tagged(&mem->alloc, is_adjacent ? 3 : 4, &left.ref)) {
		longjmp(mem->escape, mem->oom);
	}

	/* initialize tree structure fields */
	left.ref.tag = lx_tree_tag;
	right.ref = forward(left.ref);
	size.ref = forward(right.ref);
	ref_data(size.ref)->i = 1 + lx_tree_size(l) + lx_tree_size(r);
	if (lx_is_empty_tree(l)) {
		setnilref(ref_data(left.ref));
	} else {
		setref(ref_data(left.ref), l.ref);
	}
	if (lx_is_empty_tree(r)) {
		setnilref(ref_data(right.ref));
	} else {
		setref(ref_data(right.ref), r.ref);
	}

	/* handle entry */
	entry_len = lxtag_len(*ref_tag(e.ref));
	if (is_adjacent) {
		len = next_length(entry_len);
	} else if (entry_len == 1) {
		/* copy entry */
		data.ref = forward(size.ref);
		car = lx_car(e);
		lx_set_cell_data(ref_data(data.ref), car);
		*ref_tag(data.ref) = mktag(1, car.tag);
		len = 2;
	} else {
		/* link to entry */
		data.ref = forward(size.ref);
		setref(ref_data(data.ref), e.ref);
		*ref_tag(data.ref) = mktag(1, lx_list_tag);
		len = 0;
	}

	*ref_tag(size.ref) = mktag(len, lx_int_tag);
	len = next_length(len);
	*ref_tag(right.ref) = mktag(len, lx_tree_tag);
	len = next_length(len);
	*ref_tag(left.ref) = mktag(len, lx_tree_tag);

	return size;
}

/* create a balanced tree node with the given sub-trees and entry, when a
   single node has recently been added to the right sub-tree or removed from the
   left sub-tree, possibly by rotating left to restore balance */
static struct lxtree balance_left(
	struct lxmem *mem,
	struct lxtree l,
	struct lxtree r,
	struct lxlist e)
{
	struct lxtree nl, nr, rl, rll, rlr, rr;
	struct lxlist re, rle;

	if (is_balanced(l, r)) {
		return make_node(mem, l, r, e);
	}

	/* rotate left */
	assert(!lx_is_empty_tree(r));
	destructure(r, &rl, &rr, &re);
	if (is_single(rl, rr)) {
		/*
		 *     X             R
		 *    / \           / \
		 *   L   R    ->   X   RR
		 *      / \       / \
		 *    RL   RR    L   RL
		 */
		nl = make_node(mem, l, rl, e);
		return make_node(mem, nl, rr, re);
	} else {
		/*
		 *       X
		 *      / \              RL
		 *     L   R           /    \
		 *        / \   ->    X      R
		 *      RL   RR      / \    / \
		 *     /  \         L  RLL RLR RR
		 *   RLL  RLR
		 */

		assert(!lx_is_empty_tree(rl));
		destructure(rl, &rll, &rlr, &rle);

		nl = make_node(mem, l, rll, e);
		nr = make_node(mem, rlr, rr, re);

		return make_node(mem, nl, nr, rle);
	}
}

/* create a balanced tree node with the given sub-trees and entry, when a
   single node has recently been added to the left sub-tree or removed from the
   right sub-tree, possibly by rotating right to restore balance */
static struct lxtree balance_right(
	struct lxmem *mem,
	struct lxtree l,
	struct lxtree r,
	struct lxlist e)
{
	struct lxtree nl, nr, lr, ll, lrl, lrr;
	struct lxlist le, lre;

	if (is_balanced(l, r)) {
		return make_node(mem, l, r, e);
	}

	/* rotate right */
	assert(!lx_is_empty_tree(l));
	destructure(l, &ll, &lr, &le);
	if (is_single(ll, lr)) {
		/*
		 *        X         L
		 *       / \       / \
		 *      L   R -> LL   X
		 *     / \           / \
		 *   LL   LR        LR  R
		 */
		nr = make_node(mem, lr, r, e);
		return make_node(mem, ll, nr, le);
	} else {
		/*
		 *        X
		 *       / \           LR
		 *      L   R        /    \
		 *     / \     ->  L        X
		 *   LL   LR      / \      / \
		 *       /  \    LL LRL  LRR  R
		 *     LRL  LRR
		 */

		assert(!lx_is_empty_tree(lr));
		destructure(lr, &lrl, &lrr, &lre);

		nl = make_node(mem, ll, lrl, le);
		nr = make_node(mem, lrr, r, e);

		return make_node(mem, nl, nr, lre);
	}
}

struct lxtree lx_tree_cons(
	struct lxmem *mem,
	struct lxlist entry,
	struct lxtree tree)
{
	struct lxtree l, r, v;
	struct lxlist e;
	int cmp;

	if (lx_is_empty_list(entry)) { return tree; }
	if (lx_is_empty_tree(tree)) { return make_leaf_node(mem, entry); }

	destructure(tree, &l, &r, &e);
	cmp = lx_compare(lx_car(entry), lx_car(e));
	if (cmp < 0) {
		v = lx_tree_cons(mem, entry, l);
		return balance_right(mem, v, r, e);
	}
	if (cmp > 0) {
		v = lx_tree_cons(mem, entry, r);
		return balance_left(mem, l, v, e);
	}
	return make_node(mem, l, r, entry);
}

static struct lxtree extract_min(
	struct lxmem *mem,
	struct lxtree tree,
	struct lxlist *min_entry)
{
	struct lxtree l, r;
	struct lxlist e;

	assert(!lx_is_empty_tree(tree));

	destructure(tree, &l, &r, &e);
	if (lx_is_empty_tree(l)) {
		*min_entry = e;
		return r;
	} else {
		l = extract_min(mem, l, min_entry);
		return balance_left(mem, l, r, e);
	}
}

static struct lxtree extract_max(
	struct lxmem *mem,
	struct lxtree tree,
	struct lxlist *max_entry)
{
	struct lxtree l, r;
	struct lxlist e;

	assert(!lx_is_empty_tree(tree));

	destructure(tree, &l, &r, &e);
	if (lx_is_empty_tree(r)) {
		*max_entry = e;
		return l;
	} else {
		r = extract_max(mem, r, max_entry);
		return balance_right(mem, l, r, e);
	}
}

static struct lxtree remove_rec(
	struct lxmem *mem,
	union lxvalue key,
	struct lxtree tree,
	jmp_buf not_found)
{
	struct lxtree l, r;
	struct lxlist e;
	int cmp;

	if (lx_is_empty_tree(tree)) {
		longjmp(not_found, 1);
	}

	destructure(tree, &l, &r, &e);
	cmp = lx_compare(key, lx_car(e));
	if (cmp < 0) {
		l = remove_rec(mem, key, l, not_found);
		return balance_left(mem, l, r, e);
	}
	if (cmp > 0) {
		r = remove_rec(mem, key, r, not_found);
		return balance_right(mem, l, r, e);
	}

	/* zero/one child */
	if (lx_is_empty_tree(l)) { return r; }
	if (lx_is_empty_tree(r)) { return l; }

	/* two children */
	if (lx_tree_size(l) > lx_tree_size(r)) {
		l = extract_max(mem, l, &e);
	} else {
		r = extract_min(mem, r, &e);
	}
	return make_node(mem, l, r, e);
}

struct lxtree lx_tree_remove(
	struct lxmem *mem,
	union lxvalue key,
	struct lxtree tree)
{
	jmp_buf not_found;

	if (setjmp(not_found)) {
		return tree;
	} else {
		return remove_rec(mem, key, tree, not_found);
	}
}
