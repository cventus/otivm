#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

#include "ok/ok.h"

#include "lx32x4.h"
#include "../mark.h"

/*

            3
	  /   \
	1       5
      /   \   /   \
     0     2 4     6

 */
#include STATE_DEFINITION

struct lxalloc to;
struct lxvalue root, state_root;
union lxcell from_buf[length_of(state)], to_buf[length_of(state)], bitset[10];
union lxcell stack_buf[50], *stack;
unsigned char expected[sizeof bitset];

void before_each_test(void)
{
	memset(bitset, 0, sizeof bitset);
	memset(expected, 0, sizeof expected);
	memset(to_buf, 0, sizeof to_buf);
	memcpy(from_buf, state, sizeof state);
	init_tospace(&to, to_buf, length_of(to_buf));
	root = mktree(from_buf + root_cell, root_offset).value;
	state_root = mktree(state + root_cell, root_offset).value;
	stack = stack_buf + length_of(stack_buf);
}

int test_mark_cells(void)
{
	struct lxvalue n0, n1, n2, n3, n4, n5, n6;

	n3 = root;
	n1 = lx_tree_left(ref_to_tree(n3)).value;
	n5 = lx_tree_right(ref_to_tree(n3)).value;
	n0 = lx_tree_left(ref_to_tree(n1)).value;
	n2 = lx_tree_right(ref_to_tree(n1)).value;
	n4 = lx_tree_left(ref_to_tree(n5)).value;
	n6 = lx_tree_right(ref_to_tree(n5)).value;

	lx_count_refs(root, from_buf, stack, bitset);

	// n3 is an internal node
	mark_bits(expected, ref_offset(from_buf, backward(backward(n3))));
	mark_bits(expected, ref_offset(from_buf, backward(n3)));
	mark_bits(expected, ref_offset(from_buf, n3));
	mark_bits(expected, ref_offset(from_buf, forward(n3)));

	// n1 is an internal node
	mark_bits(expected, ref_offset(from_buf, backward(backward(n1))));
	mark_bits(expected, ref_offset(from_buf, backward(n1)));
	mark_bits(expected, ref_offset(from_buf, n1));
	mark_bits(expected, ref_offset(from_buf, forward(n1)));

	// n5 is an internal node
	mark_bits(expected, ref_offset(from_buf, backward(backward(n5))));
	mark_bits(expected, ref_offset(from_buf, backward(n5)));
	mark_bits(expected, ref_offset(from_buf, n5));
	mark_bits(expected, ref_offset(from_buf, forward(n5)));

	// n0 is a leaf node
	mark_bits(expected, ref_offset(from_buf, n0));
	mark_bits(expected, ref_offset(from_buf, forward(n0)));

	// n2 is a leaf node
	mark_bits(expected, ref_offset(from_buf, n2));
	mark_bits(expected, ref_offset(from_buf, forward(n2)));

	// n4 is a leaf node
	mark_bits(expected, ref_offset(from_buf, n4));
	mark_bits(expected, ref_offset(from_buf, forward(n4)));

	// n6 is a leaf node
	mark_bits(expected, ref_offset(from_buf, n6));
	mark_bits(expected, ref_offset(from_buf, forward(n6)));

	assert_mem_eq(bitset, expected, sizeof expected);
	return ok;
}

int test_compact(void)
{
	root = lx_compact(root, from_buf, &to, bitset, sizeof bitset);

	assert_eq(root, state_root);

	return ok;
}

int test_tree_nth(void)
{
	struct lxtree tree = lx_tree(root);
	assert_eq(lx_car(lx_tree_nth(tree, 0)), lx_valuei(0));
	assert_eq(lx_car(lx_tree_nth(tree, 1)), lx_valuei(1));
	assert_eq(lx_car(lx_tree_nth(tree, 2)), lx_valuei(2));
	assert_eq(lx_car(lx_tree_nth(tree, 3)), lx_valuei(3));
	assert_eq(lx_car(lx_tree_nth(tree, 4)), lx_valuei(4));
	assert_eq(lx_car(lx_tree_nth(tree, 5)), lx_valuei(5));
	assert_eq(lx_car(lx_tree_nth(tree, 6)), lx_valuei(6));
	return 0;
}

int test_tree_assoc(void)
{
	struct lxtree tree = lx_tree(root);
	assert_eq(lx_car(lx_tree_assoc(lx_valuei(0), tree)), lx_valuei(0));
	assert_eq(lx_car(lx_tree_assoc(lx_valuei(1), tree)), lx_valuei(1));
	assert_eq(lx_car(lx_tree_assoc(lx_valuei(2), tree)), lx_valuei(2));
	assert_eq(lx_car(lx_tree_assoc(lx_valuei(3), tree)), lx_valuei(3));
	assert_eq(lx_car(lx_tree_assoc(lx_valuei(4), tree)), lx_valuei(4));
	assert_eq(lx_car(lx_tree_assoc(lx_valuei(5), tree)), lx_valuei(5));
	assert_eq(lx_car(lx_tree_assoc(lx_valuei(6), tree)), lx_valuei(6));
	return 0;
}

int test_tree_cons(void)
{
	static int const numbers[] = { 0, 1, 2, 3, 4, 5, 6 };

	struct lxheap *heap;
	struct lxstate s[1];
	struct lxvalue key, index;
	struct lxtree tree;
	struct lxlist entry;
	int i, j, k, n, insert_order[length_of(numbers)];

	heap = lx_make_heap(0, 0);
	for (i = 1; i < (int)length_of(numbers); i++) {
		memcpy(insert_order, numbers, sizeof numbers);
		n = fact(i);
		for (j = 0; j < n; j++) {
			lx_start(s, heap);
			tree = lx_empty_tree();
			for (k = 0; k < i; k++) {
				key = lx_valuei(insert_order[k]);
				index = lx_valuei(k);
				entry = lx_pair(s, key, index);
				tree = lx_tree_cons(s, entry, tree);
			}
			lx_end(s, tree.value);

			for (k = 0; k < i; k++) {
				key = lx_valuei(insert_order[k]);
				index = lx_valuei(k);

				/* validate by key */
				entry = lx_tree_assoc(key, tree);
				assert_eq(lx_car(entry), key);
				assert_eq(lx_car(lx_cdr(entry)), index);

				/* validate by order */
				entry = lx_tree_nth(tree, k);
				assert_eq(lx_car(entry), index);
			}
			int_permute(i, insert_order);
		}
	}
	lx_free_heap(heap);

	return 0;
}

static void remove_in_order(struct lxheap *heap, int n, int const keys[])
{
	int i, j;
	struct lxvalue key;
	struct lxtree tree;
	struct lxlist entry;
	struct lxstate s[1];

	/* remove in every permutation */
	for (i = 1; i <= n; i++) {
		lx_start(s, heap);
		tree.value = lx_heap_value(heap);
		for (j = 0; j < i; j++) {
			key = lx_valuei(keys[j]);
			tree = lx_tree_remove(s, key, tree);
		}
		/* no end */

		assert_int_eq(lx_tree_size(tree), n - i);
		/* check deleted and remaining entries */
		for (j = 0; j < n; j++) {
			key = lx_valuei(keys[j]);
			entry = lx_tree_assoc(key, tree);
			if (j < i) {
				assert_list_eq(lx_empty_list(), entry);
			} else {
				assert_eq(lx_car(entry), key);
			}
		}
	}
}

int test_tree_remove(void)
{
	static int const numbers[] = { 0, 1, 2, 3, 4, 5, 6 };

	struct lxheap *heap;
	struct lxstate s[1];
	struct lxvalue key;
	struct lxtree tree;
	struct lxlist entry;
	int i, j, k, n;
	int insert_order[length_of(numbers)];
	int remove_order[length_of(numbers)];

	heap = lx_make_heap(0, 0);
	/* insert in every order, every length */
	for (i = 1; i < (int)length_of(numbers); i++) {
		memcpy(insert_order, numbers, sizeof numbers);
		n = fact(i); /* number of permutations of length i */
		for (j = 0; j < n; j++) {
			/* insert in every permutation */

			lx_start(s, heap);
			tree = lx_empty_tree();
			for (k = 0; k < i; k++) {
				key = lx_valuei(insert_order[k]);
				entry = lx_pair(s, key, lx_valuei(k));
				tree = lx_tree_cons(s, entry, tree);
			}
			lx_end(s, tree.value);

			/* remove for every permutation */
			memcpy(remove_order, insert_order, sizeof insert_order);
			for (k = 0; k < n; k++) {
				/* remove in every permutation */
				remove_in_order(heap, i, remove_order);
				int_permute(i, remove_order);
			}
			int_permute(i, insert_order);
		}
	}
	lx_free_heap(heap);

	return 0;
}

#define tree_a "{\n" \
	"  (a left)\n" \
	"  (b left)\n" \
	"  (d left)\n" \
	"  (e left)\n" \
	"  (g left)\n" \
	"  (h left)\n" \
	"}\n"

#define tree_b "{\n" \
	"  (c right)\n" \
	"  (e right)\n" \
	"  (f right)\n" \
	"  (h right)\n" \
	"  (i right)\n" \
	"}\n"

int test_tree_union(void)
{
	struct lxheap *heap;
	struct lxstate s[1];
	struct lxtree a, b, c;
	char const *string;
	
	heap = lx_make_heap(0, 0);

	lx_start(s, heap);
	a.value = lx_read(s, tree_a).value;
	b.value = lx_read(s, tree_b).value;
	c.value = lx_tree_union(s, a, b).value;
	string = lx_write(s, c.value).value.s;

	assert_str_eq(string,
		"{(a left) (b left) (c right)"
		" (d left) (e left) (f right)"
		" (g left) (h left) (i right)}");

	c.value = lx_tree_union(s, b, a).value;
	string = lx_write(s, c.value).value.s;

	assert_str_eq(string,
		"{(a left) (b left) (c right)"
		" (d left) (e right) (f right)"
		" (g left) (h right) (i right)}");

	lx_free_heap(heap);
	return 0;
}

int test_tree_isect(void)
{
	struct lxheap *heap;
	struct lxstate s[1];
	struct lxtree a, b, c;
	char const *string;
	
	heap = lx_make_heap(0, 0);

	lx_start(s, heap);
	a.value = lx_read(s, tree_a).value;
	b.value = lx_read(s, tree_b).value;
	c.value = lx_tree_isect(s, a, b).value;
	string = lx_write(s, c.value).value.s;

	assert_str_eq(string, "{(e left) (h left)}");

	c.value = lx_tree_isect(s, b, a).value;
	string = lx_write(s, c.value).value.s;
	assert_str_eq(string, "{(e right) (h right)}");

	lx_free_heap(heap);
	return 0;
}

int test_tree_diff(void)
{
	struct lxheap *heap;
	struct lxstate s[1];
	struct lxtree a, b, c;
	char const *string;
	
	heap = lx_make_heap(0, 0);

	lx_start(s, heap);
	a.value = lx_read(s, tree_a).value;
	b.value = lx_read(s, tree_b).value;
	c.value = lx_tree_diff(s, a, b).value;
	string = lx_write(s, c.value).value.s;

	assert_str_eq(string, "{(a left) (b left) (d left) (g left)}");

	lx_free_heap(heap);
	return 0;
}


static bool str_key_limit(struct lxlist entry, void *upper)
{
	return strcmp(lx_car(entry).s, upper) < 0;
}

int test_tree_filter(void)
{
	struct lxheap *heap;
	struct lxstate s[1];
	struct lxtree a, b, c;
	char const *string;
	
	heap = lx_make_heap(0, 0);

	lx_start(s, heap);
	a.value = lx_read(s, tree_a).value;
	b.value = lx_read(s, tree_b).value;
	c.value = lx_tree_union(s, b, a).value;
	c = lx_tree_filter(s, c, str_key_limit, "g");

	string = lx_write(s, c.value).value.s;

	assert_str_eq(string,
		"{(a left) (b left) (c right)"
		" (d left) (e right) (f right)}");

	lx_free_heap(heap);
	return 0;
}
