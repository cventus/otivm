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

/* The garbage collector must not expand a cdr-coded list. It should be able to
   cope with shared list structure without copying lists twice or expanding
   what has been cdr-coded. */
union lxcell state[] = { span(
	tag(int, adjacent),  int_data(1), /* X */
	tag(int, adjacent),  int_data(2),
	tag(int, adjacent),  int_data(3),
	tag(int, nil),       int_data(4)), span(
	tag(list, adjacent), ref_data(0, -1, 0), /* A */
	tag(list, adjacent), ref_data(1, -1, 1),
	tag(list, adjacent), ref_data(2, -1, 2),
	tag(list, nil),      ref_data(3, -1, 3)), span(
	tag(list, adjacent), ref_data(0, -2, 3), /* B */
	tag(list, adjacent), ref_data(1, -2, 2),
	tag(list, adjacent), ref_data(2, -2, 1),
	tag(list, nil),      ref_data(3, -2, 0)), span(
	tag(list, adjacent), ref_data(0, -3, 3), /* C */
	tag(list, adjacent), ref_data(1, -3, 2),
	tag(list, nil),      ref_data(2, -3, 1),
	tag(nil, nil),       int_data(-1)), span(
	tag(list, adjacent), ref_data(0, -4, 2),
	tag(list, nil),      ref_data(1, -4, 1),
	tag(list, adjacent), ref_data(2, -4, 3), /* D */
	tag(list, nil),      ref_data(3, 0, 0)
) }, from_buf[25], to_buf[27];

#define SERIALIZED_TREE_A "((1 2 3 4) (2 3 4) (3 4) (4))"
#define SERIALIZED_TREE_B "((4) (3 4) (2 3 4) (1 2 3 4))"
#define SERIALIZED_TREE_C "((4) (3 4) (2 3 4))"
#define SERIALIZED_TREE_D "((4) ((3 4) (2 3 4)))"

size_t compacted_size_A = 10;
size_t compacted_size_B = 10;
size_t compacted_size_C = 8;
size_t compacted_size_D = 9;

struct lxalloc to;

union lxvalue root_A, root_B, root_C, root_D;

union lxcell stack_buf[50], *stack;

void before_each_test(void)
{
	memcpy(from_buf, state, sizeof state);
	init_tospace(&to, to_buf, 27);

	stack = stack_buf + 50;
	root_A = lx_list(mklist(from_buf + 5, 0));
	root_B = lx_list(mklist(from_buf + 10, 0));
	root_C = lx_list(mklist(from_buf + 15, 0));
	root_D = lx_list(mklist(from_buf + 20, 2));
}

int test_tospace_should_contain_25_cells(void)
{
	return assert_int_eq(alloc_cell_count(&to), 25);
}

int test_tospace_should_contain_2_mark_cells(void)
{
	return assert_int_eq(alloc_mark_cell_count(&to), 2);
}

int test_A_count_refs_should_mark_shared_cells_twice(void)
{
	unsigned char expected[] = {
		/* First four cells are shared, both bits set */
		0xff,
		/* Second four cells are only reachable once */
		0x55
	}, bitset[sizeof expected];

	memset(bitset, 0, sizeof bitset);
	lx_count_refs(root_A, from_buf, stack, bitset);
	assert_mem_eq(bitset, expected, sizeof expected);
	return ok;
}

int test_A_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_A)];

	serialize(root_A, buf, sizeof buf);
	assert_str_eq(buf, SERIALIZED_TREE_A);

	return ok;
}

int test_A_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_A)];

	root_A = lx_compact(root_A, from_buf, &to);
	serialize(root_A, buf, sizeof buf);
	assert_str_eq(buf, SERIALIZED_TREE_A);

	return ok;
}

int test_A_compacted_tree_is_smaller(void)
{
	lx_compact(root_A, from_buf, &to);
	swap_allocation_pointers(&to);
	assert_int_eq(alloc_low_used_count(&to), compacted_size_A);
	return ok;
}

int test_B_count_refs_should_mark_shared_cells_twice(void)
{
	unsigned char expected[] = {
		/* First four cells are shared, both bits set */
		0xff, 0x00,
		/* Second four cells are only reachable once */
		0x55, 0x00, 0x00, 0x00, 0x00, 0x00
	}, bitset[sizeof expected];

	memset(bitset, 0, sizeof bitset);
	lx_count_refs(root_B, from_buf, stack, bitset);
	assert_mem_eq(bitset, expected, sizeof expected);
	return ok;
}

int test_B_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_B)];

	serialize(root_B, buf, sizeof buf);
	assert_str_eq(buf, SERIALIZED_TREE_B);

	return ok;
}

int test_B_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_B)];

	root_B = lx_compact(root_B, from_buf, &to);
	serialize(root_B, buf, sizeof buf);
	assert_str_eq(buf, SERIALIZED_TREE_B);

	return ok;
}

int test_B_compacted_tree_is_smaller(void)
{
	lx_compact(root_B, from_buf, &to);
	swap_allocation_pointers(&to);
	assert_int_eq(alloc_low_used_count(&to), compacted_size_B);
	return ok;
}

int test_C_count_refs_should_mark_shared_cells_twice(void)
{
	unsigned char expected[] = {
		/* First four cells are shared, both bits set */
		0xfc, 0x00, 0x00,
		/* third three cells are only reachable once from the root */
		0x15,

		0x00, 0x00, 0x00, 0x00
	}, bitset[sizeof expected];

	memset(bitset, 0, sizeof bitset);
	lx_count_refs(root_C, from_buf, stack, bitset);
	assert_mem_eq(bitset, expected, sizeof expected);
	return ok;
}

int test_C_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_C)];

	serialize(root_C, buf, sizeof buf);
	assert_str_eq(buf, SERIALIZED_TREE_C);

	return ok;
}

int test_C_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_C)];

	root_C = lx_compact(root_C, from_buf, &to);
	serialize(root_C, buf, sizeof buf);
	assert_str_eq(buf, SERIALIZED_TREE_C);

	return ok;
}

int test_C_compacted_tree_is_smaller(void)
{
	lx_compact(root_C, from_buf, &to);
	swap_allocation_pointers(&to);
	assert_int_eq(alloc_low_used_count(&to), compacted_size_C);
	return ok;
}

int test_D_count_refs_should_mark_shared_cells_twice(void)
{
	unsigned char expected[] = {
		/* First four cells are shared, both bits set */
		0xfc, 0x00, 0x00, 0x00,
		/* fourth span of cells are only reachable from the root */
		0x55, 0x00, 0x00, 0x00
	}, bitset[sizeof expected];

	memset(bitset, 0, sizeof bitset);
	lx_count_refs(root_D, from_buf, stack, bitset);
	assert_mem_eq(bitset, expected, sizeof expected);
	return ok;
}

int test_D_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_D)];

	serialize(root_D, buf, sizeof buf);
	assert_str_eq(buf, SERIALIZED_TREE_D);

	return ok;
}

int test_D_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE_D)];

	root_D = lx_compact(root_D, from_buf, &to);
	serialize(root_D, buf, sizeof buf);
	assert_str_eq(buf, SERIALIZED_TREE_D);

	return ok;
}

int test_D_compacted_tree_is_smaller(void)
{
	lx_compact(root_D, from_buf, &to);
	swap_allocation_pointers(&to);
	assert_int_eq(alloc_low_used_count(&to), compacted_size_D);
	return ok;
}
