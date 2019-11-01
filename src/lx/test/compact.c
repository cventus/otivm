#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ok/ok.h"

#include "lx32x4.h"

enum { ALPHA, BETA, GAMMA };

#include STATE_DEFINITION

union lxcell from_buf[length_of(state)], to_buf[length_of(state)], bitset[1];

struct lxalloc to;

struct lxvalue root, nil;

void before_each_test(void)
{
	memset(bitset, 0, sizeof bitset);
	memcpy(from_buf, state, sizeof state);
	init_tospace(&to, to_buf, length_of(to_buf));
	root = mklist(from_buf + alpha_cell, alpha_offset).value;
	nil = lx_empty_list().value;
}

int test_expected_fromspace_structure(void)
{
	assert_eq(lx_nth(lx_list(root), 0), lx_valuei(ALPHA));
	assert_eq(lx_nth(lx_list(root), 1), lx_valuei(BETA));
	assert_eq(lx_nth(lx_list(root), 2), lx_valuei(GAMMA));
	assert_eq(lx_drop(lx_list(root), 3).value, nil);

	return ok;
}

int test_expected_tospace_structure(void)
{
	root = lx_compact(root, from_buf, &to, bitset, sizeof bitset);

	assert_eq(lx_nth(lx_list(root), 0), lx_valuei(ALPHA));
	assert_eq(lx_nth(lx_list(root), 1), lx_valuei(BETA));
	assert_eq(lx_nth(lx_list(root), 2), lx_valuei(GAMMA));
	assert_eq(lx_drop(lx_list(root), 3).value, nil);

	return ok;
}

int test_compacted_list_has_adjacent_elements(void)
{
	lxtag const *t;

	root = lx_compact(root, from_buf, &to, bitset, sizeof bitset);

	/* three cells used in one span, plus leading sentinel node, tag free
	   should point at next cell to allocate */
	assert_int_eq(to.tag_free.offset, 0);
	assert_ptr_eq(ref_cell(to.tag_free), to_buf + SPAN_LENGTH);

	assert_int_eq(root.offset, 1);
	assert_ptr_eq(ref_cell(root), to_buf);

	t = ref_cell(root)->t;
	assert_int_eq(t[1], mktag(3, lx_int_tag));
	assert_int_eq(t[2], mktag(2, lx_int_tag));
	assert_int_eq(t[3], mktag(1, lx_int_tag));

	return ok;
}
