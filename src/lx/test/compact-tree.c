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
"(8 (4 (2 (1 ()) 3 ()) 6 (5 ()) 7 ()) 12 (10 (9 ()) 11 ()) 14 (13 ()) 15 ())"

#include STATE_DEFINITION

union lxcell from_buf[length_of(state)], to_buf[length_of(state)];
char bitset[80];
struct lxalloc to;

union lxvalue root;

void before_each_test(void)
{
	memset(bitset, 0, sizeof bitset);
	memcpy(from_buf, state, sizeof state);
	init_tospace(&to, to_buf, length_of(to_buf));
	root = lx_list(mklist(from_buf, root_offset));
}

int test_expected_fromspace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE)];
	assert_str_eq(serialize(root, buf, sizeof buf), SERIALIZED_TREE);
	return ok;
}

int test_expected_tospace_structure(void)
{
	char buf[2 * sizeof(SERIALIZED_TREE)];
	root = lx_compact(root, from_buf, &to, bitset, sizeof bitset);
	assert_str_eq(serialize(root, buf, sizeof buf), SERIALIZED_TREE);
	return ok;
}

int test_compacted_tree_is_smaller(void)
{
	lx_compact(root, from_buf, &to, bitset, sizeof bitset);
	assert_int_eq(ref_offset(to_buf, to.tag_free), 38);
	return ok;
}
