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
	assert_int_eq(lx_car(lx_tree_nth(root.tree, 0)).i, 0);
	assert_int_eq(lx_car(lx_tree_nth(root.tree, 1)).i, 1);
	assert_int_eq(lx_car(lx_tree_nth(root.tree, 2)).i, 2);
	assert_int_eq(lx_car(lx_tree_nth(root.tree, 3)).i, 3);
	assert_int_eq(lx_car(lx_tree_nth(root.tree, 4)).i, 4);
	assert_int_eq(lx_car(lx_tree_nth(root.tree, 5)).i, 5);
	assert_int_eq(lx_car(lx_tree_nth(root.tree, 6)).i, 6);
	return 0;
}
