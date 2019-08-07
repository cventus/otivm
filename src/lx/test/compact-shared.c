#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "ok/ok.h"

#include "lx32x4.h"
#include "../mark.h"

#include STATE_DEFINITION

/* The garbage collector must not expand a cdr-coded list. It should be able to
   cope with shared list structure without copying lists twice or expanding
   what has been cdr-coded. */
union lxcell from_buf[length_of(state)], to_buf[length_of(state)];

#define SERIALIZED_TREE_A "((1 2 3 4) (2 3 4) (3 4) (4))"
#define SERIALIZED_TREE_B "((4) (3 4) (2 3 4) (1 2 3 4))"
#define SERIALIZED_TREE_C "((4) (3 4) (2 3 4))"
#define SERIALIZED_TREE_D "((4) ((3 4) (2 3 4)))"

size_t compacted_size_A = 11;
size_t compacted_size_B = 11;
size_t compacted_size_C = 8;
size_t compacted_size_D = 10;

struct lxalloc to;

union lxvalue root_X, root_A, root_B, root_C, root_d, root_D;

union lxcell stack_buf[50], *stack;

unsigned char expected[20], bitset[sizeof expected];

void before_each_test(void)
{
	memset(bitset, 0, sizeof bitset);
	memset(expected, 0, sizeof expected);
	memset(to_buf, 0, sizeof to_buf);
	memcpy(from_buf, state, sizeof state);
	init_tospace(&to, to_buf, length_of(to_buf));
	stack = stack_buf + length_of(stack_buf);

	root_X = lx_list(mklist(from_buf + list_X_cell, list_X_offset));
	root_A = lx_list(mklist(from_buf + list_A_cell, list_A_offset));
	root_B = lx_list(mklist(from_buf + list_B_cell, list_B_offset));
	root_C = lx_list(mklist(from_buf + list_C_cell, list_C_offset));
	root_d = lx_list(mklist(from_buf + list_d_cell, list_d_offset));
	root_D = lx_list(mklist(from_buf + list_D_cell, list_D_offset));
}

int test_A_count_refs_should_mark_shared_cells_twice(void)
{
	struct lxref ref;

	lx_count_refs(root_A, from_buf, stack, bitset);

	// list X is shared by different elements of list A
	mark_shared_bits(expected, ref_offset(from_buf, ref = root_X.list.ref));
	mark_shared_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_shared_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_shared_bits(expected, ref_offset(from_buf, forward(ref)));

	// list A is only referenced once
	mark_bits(expected, ref_offset(from_buf, ref = root_A.list.ref));
	mark_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_bits(expected, ref_offset(from_buf, forward(ref)));

	assert_mem_eq(bitset, expected, sizeof expected);
	return ok;
}

int test_A_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_A)];
	assert_str_eq(serialize(root_A, buf, sizeof buf), SERIALIZED_TREE_A);
	return ok;
}

int test_A_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_A)];

	root_A = lx_compact(root_A, from_buf, &to, bitset, sizeof bitset);
	assert_str_eq(serialize(root_A, buf, sizeof buf), SERIALIZED_TREE_A);

	return ok;
}

int test_A_compacted_tree_is_smaller(void)
{
	lx_compact(root_A, from_buf, &to, bitset, sizeof bitset);
	assert_int_eq(ref_offset(to_buf, to.tag_free), compacted_size_A);
	return ok;
}

int test_B_count_refs_should_mark_shared_cells_twice(void)
{
	struct lxref ref;

	lx_count_refs(root_B, from_buf, stack, bitset);

	// list X is shared by different elements of list B
	mark_shared_bits(expected, ref_offset(from_buf, ref = root_X.list.ref));
	mark_shared_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_shared_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_shared_bits(expected, ref_offset(from_buf, forward(ref)));

	// list B is only referenced once
	mark_bits(expected, ref_offset(from_buf, ref = root_B.list.ref));
	mark_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_bits(expected, ref_offset(from_buf, forward(ref)));

	assert_mem_eq(bitset, expected, sizeof expected);

	return ok;
}

int test_B_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_B)];
	assert_str_eq(serialize(root_B, buf, sizeof buf), SERIALIZED_TREE_B);
	return ok;
}

int test_B_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_B)];

	root_B = lx_compact(root_B, from_buf, &to, bitset, sizeof bitset);
	assert_str_eq(serialize(root_B, buf, sizeof buf), SERIALIZED_TREE_B);

	return ok;
}

int test_B_compacted_tree_is_smaller(void)
{
	lx_compact(root_B, from_buf, &to, bitset, sizeof bitset);
	assert_int_eq(ref_offset(to_buf, to.tag_free), compacted_size_B);
	return ok;
}

int test_C_count_refs_should_mark_shared_cells_twice(void)
{
	struct lxref ref;

	lx_count_refs(root_C, from_buf, stack, bitset);

	// cdr(list X) is shared by different elements of list C
	ref = root_X.list.ref;
	mark_shared_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_shared_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_shared_bits(expected, ref_offset(from_buf, forward(ref)));

	// list C is only referenced once
	mark_bits(expected, ref_offset(from_buf, ref = root_C.list.ref));
	mark_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_bits(expected, ref_offset(from_buf, forward(ref)));

	assert_mem_eq(bitset, expected, sizeof expected);

	return ok;
}

int test_C_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_C)];
	assert_str_eq(serialize(root_C, buf, sizeof buf), SERIALIZED_TREE_C);
	return ok;
}

int test_C_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_C)];

	root_C = lx_compact(root_C, from_buf, &to, bitset, sizeof bitset);
	assert_str_eq(serialize(root_C, buf, sizeof buf), SERIALIZED_TREE_C);

	return ok;
}

int test_C_compacted_tree_is_smaller(void)
{
	lx_compact(root_C, from_buf, &to, bitset, sizeof bitset);
	assert_int_eq(ref_offset(to_buf, to.tag_free), compacted_size_C);
	return ok;
}

int test_D_count_refs_should_mark_shared_cells_twice(void)
{
	struct lxref ref;

	lx_count_refs(root_D, from_buf, stack, bitset);

	// list X is shared by different elements of list A
	ref = root_X.list.ref;
	mark_shared_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_shared_bits(expected, ref_offset(from_buf, ref = forward(ref)));
	mark_shared_bits(expected, ref_offset(from_buf, forward(ref)));

	// list d is only referenced once
	mark_bits(expected, ref_offset(from_buf, ref = root_d.list.ref));
	mark_bits(expected, ref_offset(from_buf, forward(ref)));

	// list D is only referenced once
	mark_bits(expected, ref_offset(from_buf, ref = root_D.list.ref));
	mark_bits(expected, ref_offset(from_buf, forward(ref)));

	assert_mem_eq(bitset, expected, sizeof expected);

	return ok;
}

int test_D_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_D)];
	assert_str_eq(serialize(root_D, buf, sizeof buf), SERIALIZED_TREE_D);
	return ok;
}

int test_D_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_D)];

	root_D = lx_compact(root_D, from_buf, &to, bitset, sizeof bitset);
	assert_str_eq(serialize(root_D, buf, sizeof buf), SERIALIZED_TREE_D);

	return ok;
}

int test_D_compacted_tree_is_smaller(void)
{
	lx_compact(root_D, from_buf, &to, bitset, sizeof bitset);
	assert_int_eq(ref_offset(to_buf, to.tag_free), compacted_size_D);
	return ok;
}
