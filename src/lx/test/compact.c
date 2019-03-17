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
	tag(int, nil),  int_data(GAMMA),
	tag(int, nil),  int_data(0xDEAD),
	tag(int, nil),  int_data(0xDEAD),
	tag(int, link), int_data(ALPHA)
), span(
	cdr_tag,        ref_data(0, 0, 1),
	tag(int, link), int_data(BETA),
	cdr_tag,        ref_data(2, -1, 0),
	tag(int, nil),  int_data(0xDEAD)
)
}, from_buf[10], to_buf[11];

struct lxspace from, to;

union lxvalue root, nil;

void before_each_test(void)
{
	memcpy(from_buf, state, sizeof state);
	init_allocspace(&from, from_buf, 10);
	init_tospace(&to, to_buf, 11);
	from.tag_free.cell = from_buf;
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
	root = lx_compact(root, &from, &to);

	assert_eq(lx_nth(root.list, 0), lx_int(ALPHA));
	assert_eq(lx_nth(root.list, 1), lx_int(BETA));
	assert_eq(lx_nth(root.list, 2), lx_int(GAMMA));
	assert_eq(lx_list(lx_drop(root.list, 3)), nil);

	return ok;
}

int test_compacted_list_has_adjacent_elements(void)
{
	lxtag const *t;

	root = lx_compact(root, &from, &to);

	/* three cells used in one span, point at next allocation */
	assert_int_eq(to.tag_free.offset, 3);
	assert_int_eq(root.list.ref.offset, 0);
	assert_ptr_eq(to.tag_free.cell, to_buf);
	assert_ptr_eq(root.list.ref.cell, to_buf);

	t = to.tag_free.cell->t;
	assert_int_eq(t[0], mktag(cdr_adjacent, lx_int_tag));
	assert_int_eq(t[1], mktag(cdr_adjacent, lx_int_tag));
	assert_int_eq(t[2], mktag(cdr_nil, lx_int_tag));

	return ok;
}
