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

enum { ALPHA, BETA, GAMMA };

/*
Binary search tree where
  - nodes are lists of 2 + n elements, or nil
  - key is in car
  - left child is cadr (reference to list/nil),
  - right child is cddr (reference to list/nil)

  (8 (4 (3 (1 ())
         . (2 ()))
      . (6 (5 ())
         . (7 ())))
   . (12 (10 (9 ())
           . (11 ()))
       . (14 (13 ())
           . (15 ()))))

After compaction every cddr should be compacted since there is no shared
structure in the tree.
*/
#define SERIALIZED_TREE \
"(8 (4 (3 (1 ()) 2 ()) 6 (5 ()) 7 ()) 12 (10 (9 ()) 11 ()) 14 (13 ()) 15 ())"
union lxcell state[] = { span(
	/* (8 (4 ...) . (12 ...)) */
	tag(int, adjacent),  int_data(8),
	tag(list, link),     ref_data(1, 0, 3),
	tag(list, nil),      ref_data(2, 5, 0),
	/* (4 (3 ...) . (6 ...)) */
	tag(int, adjacent),  int_data(4)), span(
	tag(list, link),     ref_data(0, 0, 2),
	tag(list, nil),      ref_data(1, 2, 1),
	/* (3 (1 ...) . (2 ...)) */
	tag(int, adjacent),  int_data(3),
	tag(list, link),     ref_data(3, 1, 1)), span(
	tag(list, nil),      ref_data(0, 0, 3),
	/* (1 ()) */
	tag(int, adjacent),  int_data(1),
	tag(nil, nil),       int_data(-1),
	/* (2 ()) */
	tag(int, adjacent),  int_data(2)), span(
	tag(nil, nil),       int_data(-1),
	/* (6 (5 ...) . (7 ...)) */
	tag(int, adjacent),  int_data(6),
	tag(list, link),     ref_data(2, 1, 0),
	tag(list, nil),      ref_data(3, 1, 2)), span(
	/* (5 ()) */
	tag(int, adjacent),  int_data(5),
	tag(nil, nil),       int_data(-1),
	/* (7 ()) */
	tag(int, adjacent),  int_data(7),
	tag(nil, nil),       int_data(-1)), span(
   	/* (12 (10 ...)) . (14 ...)) */
	tag(int, adjacent),  int_data(12),
	tag(list, link),     ref_data(1, 0, 3),
	tag(list, nil),      ref_data(2, 2, 2),
   	/* (10 (9 ...)) . (11 ...)) */
	tag(int, adjacent),  int_data(10)), span(
	tag(list, link),     ref_data(0, 0, 2),
	tag(list, nil),      ref_data(1, 1, 0),
   	/* (9 ()) */
	tag(int, adjacent),  int_data(9),
	tag(nil, nil),       int_data(-1)), span(
	/* (11 ()) */
	tag(int, adjacent),  int_data(11),
	tag(nil, nil),       int_data(-1),
   	/* (14 (13 ...) . (15 ...)) */
	tag(int, adjacent),  int_data(14),
	tag(list, link),     ref_data(3, 1, 1)), span(
	tag(list, nil),      ref_data(0, 0, 3),
   	/* (13 ()) */
	tag(int, adjacent),  int_data(13),
	tag(nil, nil),       int_data(-1),
   	/* (15 ()) */
	tag(int, adjacent),  int_data(15)), span(
	tag(nil, nil),       int_data(-1),
	tag(nil, nil),       int_data(-1),
	tag(nil, nil),       int_data(-1),
	tag(nil, nil),       int_data(-1)
) }, from_buf[10*SPAN_LENGTH], to_buf[10*SPAN_LENGTH], bitset[4];

struct lxalloc to;

union lxvalue root;

void before_each_test(void)
{
	memset(bitset, 0, sizeof bitset);
	memcpy(from_buf, state, sizeof state);
	init_tospace(&to, to_buf, 50);
	root = lx_list(mklist(from_buf, 0));
}

int test_tospace_should_contain_50_cells(void)
{
	return assert_int_eq(alloc_cell_count(&to), 50);
}

int test_there_should_be_4_mark_cells(void)
{
	return assert_int_eq(mark_cell_count(50), 4);
}

int test_count_refs_should_mark_all_cells_once(void)
{
	union lxcell stack[50];
	unsigned char expected[] = {
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x01
	}, bits[sizeof expected];

	memset(bits, 0, sizeof bits);
	lx_count_refs(root, from_buf, stack + 50, bits);
	assert_mem_eq(bits, expected, sizeof expected);
	return ok;
}

int test_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE)];

	serialize(root, buf, sizeof buf);
	assert(strcmp(buf, SERIALIZED_TREE) == 0);

	return ok;
}

int test_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE)];

	root = lx_compact(root, from_buf, &to, bitset, sizeof bitset);
	serialize(root, buf, sizeof buf);
	assert(strcmp(buf, SERIALIZED_TREE) == 0);

	return ok;
}

int test_compacted_tree_is_smaller(void)
{
	lx_compact(root, from_buf, &to, bitset, sizeof bitset);
	swap_allocation_pointers(&to);
	assert(alloc_low_used_count(&to) == 38);
	return ok;
}
