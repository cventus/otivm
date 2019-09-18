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
union lxvalue root, state_root;
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
	root = lx_tree(mktree(from_buf + root_cell, root_offset));
	state_root = lx_tree(mktree(state + root_cell, root_offset));
	stack = stack_buf + length_of(stack_buf);
}

int test_mark_cells(void)
{
	struct lxref n0, n1, n2, n3, n4, n5, n6;

	n3 = root.tree.ref;
	n1 = lx_tree_left(ref_to_tree(n3)).ref;
	n5 = lx_tree_right(ref_to_tree(n3)).ref;
	n0 = lx_tree_left(ref_to_tree(n1)).ref;
	n2 = lx_tree_right(ref_to_tree(n1)).ref;
	n4 = lx_tree_left(ref_to_tree(n5)).ref;
	n6 = lx_tree_right(ref_to_tree(n5)).ref;

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
	assert_eq(lx_car(lx_tree_nth(root.tree, 0)), lx_int(0));
	assert_eq(lx_car(lx_tree_nth(root.tree, 1)), lx_int(1));
	assert_eq(lx_car(lx_tree_nth(root.tree, 2)), lx_int(2));
	assert_eq(lx_car(lx_tree_nth(root.tree, 3)), lx_int(3));
	assert_eq(lx_car(lx_tree_nth(root.tree, 4)), lx_int(4));
	assert_eq(lx_car(lx_tree_nth(root.tree, 5)), lx_int(5));
	assert_eq(lx_car(lx_tree_nth(root.tree, 6)), lx_int(6));
	return 0;
}

int test_tree_assoc(void)
{
	assert_eq(lx_car(lx_tree_assoc(lx_int(0), root.tree)), lx_int(0));
	assert_eq(lx_car(lx_tree_assoc(lx_int(1), root.tree)), lx_int(1));
	assert_eq(lx_car(lx_tree_assoc(lx_int(2), root.tree)), lx_int(2));
	assert_eq(lx_car(lx_tree_assoc(lx_int(3), root.tree)), lx_int(3));
	assert_eq(lx_car(lx_tree_assoc(lx_int(4), root.tree)), lx_int(4));
	assert_eq(lx_car(lx_tree_assoc(lx_int(5), root.tree)), lx_int(5));
	assert_eq(lx_car(lx_tree_assoc(lx_int(6), root.tree)), lx_int(6));
	return 0;
}

static union lxvalue
insert_into_tree(struct lxmem *mem, union lxvalue val, va_list ap)
{
	struct lx_list entry;
	struct lx_tree tree;
	int const *keys;
	int i, n;

	(void)val;
	n = va_arg(ap, int);
	keys = va_arg(ap, int const *);
	tree = lx_empty_tree();

	for (i = 0; i < n; i++) {
		entry = lx_pair(mem, lx_int(keys[i]), lx_int(i));
		tree = lx_tree_cons(mem, entry, tree);
	}

	return lx_tree(tree);
}

static union lxvalue
remove_from_tree(struct lxmem *mem, union lxvalue val, va_list ap)
{
	union lxvalue key;
	struct lxtree tree, modified_tree;
	int const *keys;
	int i, n;

	n = va_arg(ap, int);
	keys = va_arg(ap, int const *);
	modified_tree = tree = lx_car(val.list).tree;

	for (i = 0; i < n; i++) {
		key = lx_int(keys[i]);
		modified_tree = lx_tree_remove(mem, key, modified_tree);
	}
	return lx_list(lx_pair(mem, lx_tree(tree), lx_tree(modified_tree)));
}

static union lxvalue
duplicate_root(struct lxmem *mem, union lxvalue val, va_list ap)
{
	(void)ap;
	return lx_list(lx_pair(mem, val, val));
}

int test_tree_cons(void)
{
	static int const numbers[] = { 0, 1, 2, 3, 4, 5, 6 };

	struct lxheap *heap;
	union lxvalue key, index;
	struct lxresult result;
	struct lxtree tree;
	struct lxlist entry;
	int i, j, k, n, a[length_of(numbers)];

	heap = lx_make_heap(0, 0);
	for (i = 1; i < (int)length_of(numbers); i++) {
		memcpy(a, numbers, sizeof numbers);
		n = fact(i);
		for (j = 0; j < n; j++) {
			result = lx_modifyl(heap, insert_into_tree, i, a);
			tree = result.value.tree;
			assert_tag_eq(tree.tag, lx_tree_tag);

			for (k = 0; k < i; k++) {
				key = lx_int(a[k]);
				index = lx_int(k);

				/* validate by key */
				entry = lx_tree_assoc(key, tree);
				assert_eq(lx_car(entry), key);
				assert_eq(lx_car(lx_cdr(entry)), index);

				/* validate by order */
				entry = lx_tree_nth(tree, k);
				assert_eq(lx_car(entry), index);
			}
			int_permute(i, a);
		}
	}
	lx_free_heap(heap);

	return 0;
}

int test_tree_remove(void)
{
	static int const numbers[] = { 0, 1, 2, 3, 4, 5, 6 };

	struct lxheap *heap;
	union lxvalue key;
	struct lxresult result;
	struct lxtree tree;
	struct lx_list entry;
	int i, j, k, l, m, n, a[length_of(numbers)], b[length_of(numbers)];

	heap = lx_make_heap(0, 0);
	/* insert in every order, every length */
	for (i = 1; i < (int)length_of(numbers); i++) {
		memcpy(a, numbers, sizeof numbers);
		n = fact(i); /* number of permutations of length i */
		for (j = 0; j < n; j++) {
			/* insert in every permutation */
			lx_modifyl(heap, insert_into_tree, i, a);
			lx_modifyl(heap, duplicate_root);

			/* remove for every permutation */
			memcpy(b, a, sizeof a);
			for (k = 0; k < n; k++) {
				/* remove in every permutation */
				for (l = 1; l <= i; l++) {
					result = lx_modifyl(heap, remove_from_tree, l, b);
					assert_tag_eq(result.value.tag, lx_list_tag);
					assert_int_eq(lx_length(result.value.list), 2);
					tree = lx_nth(result.value.list, 1).tree;
					assert_tag_eq(tree.tag, lx_tree_tag);

					assert_int_eq(lx_tree_size(tree), i - l);
					/* check deleted and remaining entries */
					for (m = 0; m < i; m++) {
						key = lx_int(b[m]);
						entry = lx_tree_assoc(key, tree);
						if (m < l) {
							assert_list_eq(lx_empty_list(), entry);
						} else {
							assert_eq(lx_car(entry), key);
						}
					}
				}
				int_permute(i, b);
			}
			int_permute(i, a);
		}
	}
	lx_free_heap(heap);

	return 0;
}

static union lxvalue parse(struct lxmem *mem, union lxvalue val, va_list ap)
{
	struct lxread res;
	char const *p;

	p = va_arg(ap, char const *);
	res = lx_read(mem, p);
	return lx_list(lx_cons(mem, res.value, val.list));
}

static union lxvalue tree_op(struct lxmem *mem, union lxvalue val, va_list ap)
{
	typedef struct lxtree op(struct lxmem *, struct lxtree, struct lxtree);
	struct lxtree a, b;
	op *f;

	a = lx_nth(val.list, 0).tree;
	b = lx_nth(val.list, 1).tree;
	f = va_arg(ap, op *);

	return lx_list(lx_cons(mem, lx_tree(f(mem, a, b)), lx_drop(val.list, 2)));
}

static union lxvalue filter_op(struct lxmem *mem, union lxvalue val, va_list ap)
{
	typedef bool op(struct lxlist, void *);
	struct lxtree a, b;
	op *f;
	void *arg;

	a = lx_car(val.list).tree;
	f = va_arg(ap, op *);
	arg = va_arg(ap, void *);

	b = lx_tree_filter(mem, a, f, arg);

	return lx_list(lx_cons(mem, lx_tree(b), lx_cdr(val.list)));
}

static union lxvalue stringify(struct lxmem *mem, union lxvalue val, va_list ap)
{
	union lxvalue str;
	(void)ap;
	str = lx_write(mem, lx_car(val.list));
	return lx_list(lx_cons(mem, str, lx_cdr(val.list)));
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
	char const *string;
	
	heap = lx_make_heap(0, 0);

	lx_modifyl(heap, parse, tree_b);
	lx_modifyl(heap, parse, tree_a);
	lx_modifyl(heap, tree_op, lx_tree_union);

	string = lx_car(lx_modifyl(heap, stringify).value.list).s;
	assert_str_eq(string,
		"{(a left) (b left) (c right)"
		" (d left) (e left) (f right)"
		" (g left) (h left) (i right)}");

	lx_modifyl(heap, parse, tree_a);
	lx_modifyl(heap, parse, tree_b);
	lx_modifyl(heap, tree_op, lx_tree_union);

	string = lx_car(lx_modifyl(heap, stringify).value.list).s;
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
	char const *string;
	
	heap = lx_make_heap(0, 0);

	lx_modifyl(heap, parse, tree_b);
	lx_modifyl(heap, parse, tree_a);
	lx_modifyl(heap, tree_op, lx_tree_isect);

	string = lx_car(lx_modifyl(heap, stringify).value.list).s;
	assert_str_eq(string, "{(e left) (h left)}");

	lx_modifyl(heap, parse, tree_a);
	lx_modifyl(heap, parse, tree_b);
	lx_modifyl(heap, tree_op, lx_tree_isect);

	string = lx_car(lx_modifyl(heap, stringify).value.list).s;
	assert_str_eq(string, "{(e right) (h right)}");

	lx_free_heap(heap);
	return 0;
}

int test_tree_diff(void)
{
	struct lxheap *heap;
	char const *string;
	
	heap = lx_make_heap(0, 0);

	lx_modifyl(heap, parse, tree_b);
	lx_modifyl(heap, parse, tree_a);
	lx_modifyl(heap, tree_op, lx_tree_diff);

	string = lx_car(lx_modifyl(heap, stringify).value.list).s;

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
	char const *string;
	
	heap = lx_make_heap(0, 0);

	lx_modifyl(heap, parse, tree_a);
	lx_modifyl(heap, parse, tree_b);
	lx_modifyl(heap, tree_op, lx_tree_union);
	lx_modifyl(heap, filter_op, str_key_limit, "g");

	string = lx_car(lx_modifyl(heap, stringify).value.list).s;
	assert_str_eq(string,
		"{(a left) (b left) (c right)"
		" (d left) (e right) (f right)}");

	lx_free_heap(heap);
	return 0;
}
