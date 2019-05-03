#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ok/ok.h"

#include "lx32x4.h"

enum { ALPHA, BETA, GAMMA };

/* Two spans, one conses in each and one (ALPHA) that is in both */
union lxcell state[] = {
   span(
	int_tag(1), int_data(GAMMA),
	int_tag(1), int_data(0xDEAD),
	int_tag(1), int_data(0xDEAD),
	int_tag(0), int_data(ALPHA)
), span(
	cdr_tag,    ref_data(0, 0, 1),
	int_tag(0), int_data(BETA),
	cdr_tag,    ref_data(2, -1, 0),
	int_tag(1), int_data(0xDEAD)
)
}, from_buf[2*SPAN_LENGTH], to_buf[2*SPAN_LENGTH], bitset[1];

struct lxalloc to;

union lxvalue root, nil;

void before_each_test(void)
{
	memset(bitset, 0, sizeof bitset);
	memcpy(from_buf, state, sizeof state);
	init_tospace(&to, to_buf, 11);
	root = lx_list(mklist(from_buf, 3));
	nil = lx_list(lx_empty_list());
}

int test_expected_fromspace_structure(void)
{
	assert_eq(lx_nth(root.list, 0), lx_int(ALPHA));
	assert_eq(lx_nth(root.list, 1), lx_int(BETA));
	assert_eq(lx_nth(root.list, 2), lx_int(GAMMA));
	assert_eq(lx_list(lx_drop(root.list, 3)), nil);

	return ok;
}

int test_expected_tospace_structure(void)
{
	root = lx_compact(root, from_buf, &to, bitset, sizeof bitset);

	assert_eq(lx_nth(root.list, 0), lx_int(ALPHA));
	assert_eq(lx_nth(root.list, 1), lx_int(BETA));
	assert_eq(lx_nth(root.list, 2), lx_int(GAMMA));
	assert_eq(lx_list(lx_drop(root.list, 3)), nil);

	return ok;
}

int test_compacted_list_has_adjacent_elements(void)
{
	lxtag const *t;

	root = lx_compact(root, from_buf, &to, bitset, sizeof bitset);

	/* three cells used in one span, plus leading sentinel node, tag free
	   should point at next cell to allocate */
	assert_int_eq(to.tag_free.offset, 0);
	assert_ptr_eq(to.tag_free.cell, to_buf + SPAN_LENGTH);

	assert_int_eq(root.list.ref.offset, 1);
	assert_ptr_eq(root.list.ref.cell, to_buf);

	t = root.list.ref.cell->t;
	assert_int_eq(t[1], mktag(3, lx_int_tag));
	assert_int_eq(t[2], mktag(2, lx_int_tag));
	assert_int_eq(t[3], mktag(1, lx_int_tag));

	return ok;
}
