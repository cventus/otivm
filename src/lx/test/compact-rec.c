#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ok/ok.h"

#include "lx32x4.h"

#include STATE_DEFINITION

union lxcell from_buf[length_of(state)], to_buf[length_of(state)], bitset[1];

struct lxalloc to;
union lxvalue root;

void before_each_test(void)
{
	memset(bitset, 0, sizeof bitset);
	memset(to_buf, 0, sizeof to_buf);
	memcpy(from_buf, state, sizeof state);
	init_tospace(&to, to_buf, 2*SPAN_LENGTH);
	root = lx_list(mklist(from_buf + root_cell, root_offset));
}

int test_recursively_defined_structure_is_properly_copied(void)
{
	union lxvalue first, second, third, fourth;

	first = lx_compact(root, from_buf, &to, bitset, sizeof bitset);

	assert_tag_eq(first.tag, lx_list_tag);

	second = lx_list(lx_cdr(first.list));
	third = lx_list(lx_cdr(second.list));
	fourth = lx_list(lx_cdr(third.list));

	/* circular when following cdr */
	assert_eq(first, fourth);

	/* circular when following car */
	assert_eq(first, lx_car(first.list));
	assert_eq(second, lx_car(second.list));
	assert_eq(third, lx_car(third.list));

	return ok;
}
