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

struct split {
	struct lxmap l, r;
	struct lxlist e;
};

static struct lxlist map_as_list(struct lxmap map)
{
	struct lxlist result;
	result.value = map.value;
	result.value.tag = lx_list_tag;
	return result;
}

static void destructure(
	struct lxmap map,
	struct lxmap *left,
	struct lxmap *right,
	struct lxlist *entry)
{
	struct lxlist list;
	union lxcell const *sz;
	assert(!lx_is_empty_map(map));
	list = map_as_list(map);
	*entry = lx_cdr(list);
	if (is_leaf_node(map)) {
		*left = *right = lx_empty_map();
	} else {
		list.value = backward(list.value);
		sz = ref_data(list.value);
		*right = isnilref(sz) ? lx_empty_map() : deref_map(sz);
		list.value = backward(list.value);
		sz = ref_data(list.value);
		*left = isnilref(sz) ? lx_empty_map() : deref_map(sz);
	}
}

static bool is_balanced(struct lxmap a, struct lxmap b)
{
	/* No risk of size_t overflow:
	 * - each map node requires four tagged cells
	 * - for 16 bit
	 *   - 2 segments per map node
	 *   - less than 2^14/3 map nodes in a heap
	 *   -> 3 * (size(a) + 1) < 3*(2^14/3 + 1) < 2^16
	 * - for 32 bit
	 *   - 1 segments per map node
	 *   - less than 2^31/5 map nodes in a heap
	 *   -> 3 * (size(a) + 1) < 3*(2^31/5 + 1) < 2^32
	 * - for 64 bit
	 *   - two map nodes per segment
	 *   - less than 2^64/9 map nodes in a heap
	 *   -> 3 * (size(a) + 1) < 3*(2^64/5 + 1) = 2^64/3 + 3 < 2^64
	 */
	/* delta = 3 */
	return 3 * (lx_map_size(a) + 1) >= lx_map_size(b) + 1;
}

static bool is_single(struct lxmap a, struct lxmap b)
{
	/* if 3*(size(x) + 1) doesn't overflow, neither does 2*size(b) */
	/* gamma = 2 */
	return lx_map_size(a) + 1 < 2 * (lx_map_size(b) + 1);
}

static struct lxmap make_leaf_node(struct lxstate *s, struct lxlist entry)
{
	struct lxvalue tuple;
	assert(!lx_is_empty_list(entry));
	tuple = lx_cons(s, lx_valuei(1), entry).value;
	tuple.tag = lx_map_tag;
	return ref_to_map(tuple);
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

static struct lxmap make_node(
	struct lxstate *s,
	struct lxmap l,
	struct lxmap r,
	struct lxlist e)
{
	struct lxmap size, left, right, data;
	lxint len, entry_len;
	bool is_adjacent;
	struct lxvalue car;

	assert(!lx_is_empty_list(e));
	assert(l.value.tag == lx_map_tag);
	assert(r.value.tag == lx_map_tag);
	assert(e.value.tag == lx_list_tag);

	if (lx_is_empty_map(l) && lx_is_empty_map(r)) {
		return make_leaf_node(s, e);
	}

	/* allocate node */
	is_adjacent = ref_eq(e.value, s->alloc.tag_free);
	if (lx_reserve_tagged(&s->alloc, is_adjacent ? 3 : 4, &left.value)) {
		lx_handle_out_of_memory(s);
	}

	/* initialize map structure fields */
	left.value.tag = lx_map_tag;
	right.value = forward(left.value);
	size.value = forward(right.value);
	ref_data(size.value)->i = 1 + lx_map_size(l) + lx_map_size(r);
	if (lx_is_empty_map(l)) {
		setnilref(ref_data(left.value));
	} else {
		setref(ref_data(left.value), l.value);
	}
	if (lx_is_empty_map(r)) {
		setnilref(ref_data(right.value));
	} else {
		setref(ref_data(right.value), r.value);
	}

	/* handle entry */
	entry_len = lxtag_len(*ref_tag(e.value));
	if (is_adjacent) {
		len = next_length(entry_len);
	} else if (entry_len == 1) {
		/* copy entry */
		data.value = forward(size.value);
		car = lx_car(e);
		lx_set_cell_data(ref_data(data.value), car);
		*ref_tag(data.value) = mktag(1, car.tag);
		len = 2;
	} else {
		/* link to entry */
		data.value = forward(size.value);
		setref(ref_data(data.value), e.value);
		*ref_tag(data.value) = mktag(1, lx_list_tag);
		len = 0;
	}

	*ref_tag(size.value) = mktag(len, lx_int_tag);
	len = next_length(len);
	*ref_tag(right.value) = mktag(len, lx_map_tag);
	len = next_length(len);
	*ref_tag(left.value) = mktag(len, lx_map_tag);

	return size;
}

/* create a balanced tree node with the given sub-trees and entry, when a
   single node has recently been added to the right sub-tree or removed from the
   left sub-tree, possibly by rotating left to restore balance */
static struct lxmap balance_left(
	struct lxstate *s,
	struct lxmap l,
	struct lxmap r,
	struct lxlist e)
{
	struct lxmap nl, nr, rl, rll, rlr, rr;
	struct lxlist re, rle;

	if (is_balanced(l, r)) {
		return make_node(s, l, r, e);
	}

	/* rotate left */
	assert(!lx_is_empty_map(r));
	destructure(r, &rl, &rr, &re);
	if (is_single(rl, rr)) {
		/*
		 *     X             R
		 *    / \           / \
		 *   L   R    ->   X   RR
		 *      / \       / \
		 *    RL   RR    L   RL
		 */
		nl = make_node(s, l, rl, e);
		return make_node(s, nl, rr, re);
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

		assert(!lx_is_empty_map(rl));
		destructure(rl, &rll, &rlr, &rle);

		nl = make_node(s, l, rll, e);
		nr = make_node(s, rlr, rr, re);

		return make_node(s, nl, nr, rle);
	}
}

/* create a balanced tree node with the given sub-trees and entry, when a
   single node has recently been added to the left sub-tree or removed from the
   right sub-tree, possibly by rotating right to restore balance */
static struct lxmap balance_right(
	struct lxstate *s,
	struct lxmap l,
	struct lxmap r,
	struct lxlist e)
{
	struct lxmap nl, nr, lr, ll, lrl, lrr;
	struct lxlist le, lre;

	if (is_balanced(l, r)) {
		return make_node(s, l, r, e);
	}

	/* rotate right */
	assert(!lx_is_empty_map(l));
	destructure(l, &ll, &lr, &le);
	if (is_single(ll, lr)) {
		/*
		 *        X         L
		 *       / \       / \
		 *      L   R -> LL   X
		 *     / \           / \
		 *   LL   LR        LR  R
		 */
		nr = make_node(s, lr, r, e);
		return make_node(s, ll, nr, le);
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

		assert(!lx_is_empty_map(lr));
		destructure(lr, &lrl, &lrr, &lre);

		nl = make_node(s, ll, lrl, le);
		nr = make_node(s, lrr, r, e);

		return make_node(s, nl, nr, lre);
	}
}

size_t lx_map_size(struct lxmap map)
{
	return lx_is_empty_map(map) ? 0 : (size_t)ref_data(map.value)->i;
}

struct lxlist lx_map_entry(struct lxmap map)
{
	return lx_cdr(map_as_list(map));
}

struct lxmap lx_map_left(struct lxmap map)
{
	return is_leaf_node(map)
		? lx_empty_map()
		: deref_map(ref_data(backward(backward(map.value))));
}

struct lxmap lx_map_right(struct lxmap map)
{
	return is_leaf_node(map)
		? lx_empty_map()
		: deref_map(ref_data(backward(map.value)));
}

struct lxlist lx_map_nth(struct lxmap map, lxint n)
{
	struct lxmap t, l, r;
	struct lxlist e;
	size_t sz;

	if (lx_is_empty_map(map)) {
		return lx_empty_list();
	}

	t = map;
	do {
		destructure(t, &l, &r, &e);
		sz = lx_map_size(l);
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

struct lxlist lx_map_assoc(struct lxmap map, struct lxvalue key)
{
	int cmp;
	struct lxmap t, l, r;
	struct lxlist e;

	t = map;
	while (!lx_is_empty_map(t)) {
		destructure(t, &l, &r, &e);
		cmp = lx_compare(key, lx_car(e));
		if (cmp < 0) t = l;
		else if (cmp > 0) t = r;
		else return e;
	}
	return lx_empty_list();
}

struct lxvalue lx_map_ref(struct lxmap map, struct lxvalue key)
{
	struct lxlist e;

	e = lx_map_assoc(map, key);
	if (lx_is_empty_list(e)) {
		return lx_nil();
	} else {
		return lx_nth(e, 1);
	}
}

bool lx_map_has(struct lxmap map, struct lxvalue key, struct lxvalue *value)
{
	struct lxlist e;

	e = lx_map_assoc(map, key);
	if (lx_is_empty_list(e)) {
		if (value) { *value = lx_nil(); }
		return false;
	} else {
		if (value) { *value = lx_nth(e, 1); }
		return true;
	}
}

struct lxmap lx_map_set(
	struct lxstate *s,
	struct lxmap map,
	struct lxvalue key,
	struct lxvalue value)
{
	struct lxmap l, r, v;
	struct lxlist e;
	int cmp;

	if (lx_is_empty_map(map)) {
		return make_leaf_node(s, lx_pair(s, key, value));
	}

	destructure(map, &l, &r, &e);
	cmp = lx_compare(key, lx_car(e));
	if (cmp < 0) {
		v = lx_map_set(s, l, key, value);
		return balance_right(s, v, r, e);
	}
	if (cmp > 0) {
		v = lx_map_set(s, r, key, value);
		return balance_left(s, l, v, e);
	}
	return make_node(s, l, r, lx_pair(s, key, value));
}

static struct lxmap extract_min(
	struct lxstate *s,
	struct lxmap map,
	struct lxlist *min_entry)
{
	struct lxmap l, r;
	struct lxlist e;

	assert(!lx_is_empty_map(map));

	destructure(map, &l, &r, &e);
	if (lx_is_empty_map(l)) {
		*min_entry = e;
		return r;
	} else {
		l = extract_min(s, l, min_entry);
		return balance_left(s, l, r, e);
	}
}

static struct lxmap extract_max(
	struct lxstate *s,
	struct lxmap map,
	struct lxlist *max_entry)
{
	struct lxmap l, r;
	struct lxlist e;

	assert(!lx_is_empty_map(map));

	destructure(map, &l, &r, &e);
	if (lx_is_empty_map(r)) {
		*max_entry = e;
		return l;
	} else {
		r = extract_max(s, r, max_entry);
		return balance_right(s, l, r, e);
	}
}

static struct lxmap remove_rec(
	struct lxstate *s,
	struct lxvalue key,
	struct lxmap map,
	jmp_buf not_found)
{
	struct lxmap l, r;
	struct lxlist e;
	int cmp;

	if (lx_is_empty_map(map)) {
		longjmp(not_found, 1);
	}

	destructure(map, &l, &r, &e);
	cmp = lx_compare(key, lx_car(e));
	if (cmp < 0) {
		l = remove_rec(s, key, l, not_found);
		return balance_left(s, l, r, e);
	}
	if (cmp > 0) {
		r = remove_rec(s, key, r, not_found);
		return balance_right(s, l, r, e);
	}

	/* zero/one child */
	if (lx_is_empty_map(l)) { return r; }
	if (lx_is_empty_map(r)) { return l; }

	/* two children */
	if (lx_map_size(l) > lx_map_size(r)) {
		l = extract_max(s, l, &e);
	} else {
		r = extract_min(s, r, &e);
	}
	return make_node(s, l, r, e);
}

struct lxmap lx_map_remove(
	struct lxstate *s,
	struct lxmap map,
	struct lxvalue key)
{
	jmp_buf not_found;

	if (setjmp(not_found)) {
		return map;
	} else {
		return remove_rec(s, key, map, not_found);
	}
}

static struct lxmap join_left(
	struct lxstate *s,
	struct lxmap l,
	struct lxmap r,
	struct lxlist e)
{
	struct lxmap rr, rl;
	struct lxlist re;

  	if (is_balanced(l, r)) {
		return make_node(s, l, r, e);
	} else {
		destructure(r, &rl, &rr, &re);
		rl = join_left(s, l, rl, e);
		return balance_right(s, rl, rr, re);
	}
}

static struct lxmap join_right(
	struct lxstate *s,
	struct lxmap l,
	struct lxmap r,
	struct lxlist e)
{
	struct lxmap lr, ll;
	struct lxlist le;

  	if (is_balanced(l, r)) {
		return make_node(s, l, r, e);
	} else {
		destructure(l, &ll, &lr, &le);
		lr = join_right(s, lr, r, e);
		return balance_left(s, ll, lr, le);
	}
}

/* create a tree from trees `l` and `r`, and entry `e` where the keys of `l`
   are smaller then the key of `e` and the keys of `r` are greater than the key
   of `e`. */
static struct lxmap join(
	struct lxstate *s,
	struct lxmap l,
	struct lxmap r,
	struct lxlist e)
{
	size_t lsz, rsz;

	lsz = lx_map_size(l);
	rsz = lx_map_size(r);

	if (lsz > rsz) {
		return join_right(s, l, r, e);
	} else if (lsz < rsz) {
		return join_left(s, l, r, e);
	} else {
		return make_node(s, l, r, e);
	}
}

/* join two trees where each key in `l` is smaller than each key in `r` */
static struct lxmap join2(struct lxstate *s, struct lxmap l, struct lxmap r)
{
	struct lxlist e;

	if (lx_is_empty_map(l)) return r;
	l = extract_max(s, l, &e);
	return join(s, l, r, e);
}

/* Split tree into two trees: .l with entries less than `key` and .r
   with entries greater than `key`. If an entry with the key is found
   store it in .e, otherwise it is empty. */
static struct split split(
	struct lxstate *state,
	struct lxmap map,
	struct lxvalue key)
{
	struct split s;
	struct lxmap l, r;
	struct lxlist e;
	int cmp;

	if (lx_is_empty_map(map)) {
		s.l = s.r = lx_empty_map();
		s.e = lx_empty_list();
	} else {
		destructure(map, &l, &r, &e);
		cmp = lx_compare(key, lx_car(e));
		if (cmp < 0) {
			s = split(state, l, key);
			s.r = join(state, s.r, r, e);
		} else if (cmp > 0) {
			s = split(state, r, key);
			s.l = join(state, l, s.l, e);
		} else {
			s.l = l;
			s.r = r;
			s.e = e;
		}
	}
	return s;
}

size_t lx_set_size(struct lxmap map)
{
	return lx_is_empty_map(map) ? 0 : (size_t)ref_data(map.value)->i;
}

struct lxmap lx_map_union(
	struct lxstate *state,
	struct lxmap lhs,
	struct lxmap rhs)
{
	struct split s;
	struct lxmap l, r;
	struct lxlist e;

	if (lx_is_empty_map(lhs)) { return rhs; }
	if (lx_is_empty_map(rhs)) { return lhs; }

	destructure(rhs, &l, &r, &e);
	s = split(state, lhs, lx_car(e));
	l = lx_map_union(state, s.l, l);
	r = lx_map_union(state, s.r, r);
	/* prefer entry(lhs) over entry(rhs) */
	if (!lx_is_empty_list(s.e)) { e = s.e; }
	return join(state, l, r, e);
}

struct lxmap lx_map_isect(
	struct lxstate *state,
	struct lxmap lhs,
	struct lxmap rhs)
{
	struct split s;
	struct lxmap l, r;
	struct lxlist e;

	if (lx_is_empty_map(lhs) || lx_is_empty_map(rhs)) {
		return lx_empty_map();
	}
	destructure(rhs, &l, &r, &e);
	s = split(state, lhs, lx_car(e));
	l = lx_map_isect(state, s.l, l);
	r = lx_map_isect(state, s.r, r);
	if (lx_is_empty_list(s.e)) {
		return join2(state, l, r);
	} else {
		return join(state, l, r, s.e); /* ignore entry(rhs) */
	}
}

struct lxmap lx_map_diff(
	struct lxstate *state,
	struct lxmap lhs,
	struct lxmap rhs)
{
	struct split s;
	struct lxmap l, r;
	struct lxlist e;

	if (lx_is_empty_map(lhs)) { return lx_empty_map(); }
	if (lx_is_empty_map(rhs)) { return lhs; }

	destructure(rhs, &l, &r, &e);
	s = split(state, lhs, lx_car(e));
	l = lx_map_diff(state, s.l, l);
	r = lx_map_diff(state, s.r, r);
	return join2(state, l, r);
}

struct lxmap lx_map_filter(
	struct lxstate *s,
	struct lxmap map,
	bool predicate(struct lxvalue key, struct lxvalue value, void *),
	void *param)
{
	struct lxmap l, r;
	struct lxlist e;

	if (lx_is_empty_map(map)) { return lx_empty_map(); }
	destructure(map, &l, &r, &e);
	l = lx_map_filter(s, l, predicate, param);
	r = lx_map_filter(s, r, predicate, param);
	if (predicate(lx_car(e), lx_nth(e, 1), param)) {
		return join(s, l, r, e);
	} else {
		return join2(s, l, r);
	}
}
