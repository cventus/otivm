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

static struct lxvalue
insert_into_tree(struct lxstate *s, struct lxvalue val, va_list ap)
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
		entry = lx_pair(s, lx_valuei(keys[i]), lx_valuei(i));
		tree = lx_tree_cons(s, entry, tree);
	}

	return tree.value;
}

static struct lxvalue
remove_from_tree(struct lxstate *s, struct lxvalue val, va_list ap)
{
	struct lxvalue key;
	struct lxtree tree, modified_tree;
	int const *keys;
	int i, n;

	n = va_arg(ap, int);
	keys = va_arg(ap, int const *);
	modified_tree = tree = lx_tree(lx_car(lx_list(val)));

	for (i = 0; i < n; i++) {
		key = lx_valuei(keys[i]);
		modified_tree = lx_tree_remove(s, key, modified_tree);
	}
	return lx_pair(s, tree.value, modified_tree.value).value;
}

static struct lxvalue
duplicate_root(struct lxstate *s, struct lxvalue val, va_list ap)
{
	(void)ap;
	return lx_pair(s, val, val).value;
}

int test_tree_cons(void)
{
	static int const numbers[] = { 0, 1, 2, 3, 4, 5, 6 };

	struct lxheap *heap;
	struct lxvalue key, index;
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
			tree = lx_tree(result.value);
			assert_tag_eq(tree.value.tag, lx_tree_tag);

			for (k = 0; k < i; k++) {
				key = lx_valuei(a[k]);
				index = lx_valuei(k);

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
	struct lxvalue key;
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
					assert_int_eq(lx_length(lx_list(result.value)), 2);
					tree = lx_tree(lx_nth(lx_list(result.value), 1));
					assert_tag_eq(tree.value.tag, lx_tree_tag);

					assert_int_eq(lx_tree_size(tree), i - l);
					/* check deleted and remaining entries */
					for (m = 0; m < i; m++) {
						key = lx_valuei(b[m]);
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

static struct lxvalue parse(struct lxstate *s, struct lxvalue val, va_list ap)
{
	struct lxread res;
	char const *p;

	p = va_arg(ap, char const *);
	res = lx_read(s, p);
	return lx_cons(s, res.value, lx_list(val)).value;
}

static struct lxvalue tree_op(struct lxstate *s, struct lxvalue val, va_list ap)
{
	typedef struct lxtree op(struct lxstate *, struct lxtree, struct lxtree);
	struct lxtree a, b;
	op *f;

	a = lx_tree(lx_nth(lx_list(val), 0));
	b = lx_tree(lx_nth(lx_list(val), 1));
	f = va_arg(ap, op *);

	return lx_cons(s, f(s, a, b).value, lx_drop(lx_list(val), 2)).value;
}

static struct lxvalue filter_op(struct lxstate *s, struct lxvalue val, va_list ap)
{
	typedef bool op(struct lxlist, void *);
	struct lxtree a, b;
	op *f;
	void *arg;

	a = lx_tree(lx_car(lx_list(val)));
	f = va_arg(ap, op *);
	arg = va_arg(ap, void *);

	b = lx_tree_filter(s, a, f, arg);

	return lx_cons(s, b.value, lx_cdr(lx_list(val))).value;
}

static struct lxvalue stringify(struct lxstate *s, struct lxvalue val, va_list ap)
{
	struct lxstring str;
	(void)ap;
	str = lx_write(s, lx_car(lx_list(val)));
	return lx_cons(s, str.value, lx_cdr(lx_list(val))).value;
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

	string = lx_car(lx_list(lx_modifyl(heap, stringify).value)).s;
	assert_str_eq(string,
		"{(a left) (b left) (c right)"
		" (d left) (e left) (f right)"
		" (g left) (h left) (i right)}");

	lx_modifyl(heap, parse, tree_a);
	lx_modifyl(heap, parse, tree_b);
	lx_modifyl(heap, tree_op, lx_tree_union);

	string = lx_car(lx_list(lx_modifyl(heap, stringify).value)).s;
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

	string = lx_car(lx_list(lx_modifyl(heap, stringify).value)).s;
	assert_str_eq(string, "{(e left) (h left)}");

	lx_modifyl(heap, parse, tree_a);
	lx_modifyl(heap, parse, tree_b);
	lx_modifyl(heap, tree_op, lx_tree_isect);

	string = lx_car(lx_list(lx_modifyl(heap, stringify).value)).s;
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

	string = lx_car(lx_list(lx_modifyl(heap, stringify).value)).s;

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

	string = lx_car(lx_list(lx_modifyl(heap, stringify).value)).s;
	assert_str_eq(string,
		"{(a left) (b left) (c right)"
		" (d left) (e right) (f right)}");

	lx_free_heap(heap);
	return 0;
}
