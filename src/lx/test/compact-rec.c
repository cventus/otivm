#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ok/ok.h"

#include "lx32x4.h"

union lxcell state[] = {
   span(
	tag(list, adjacent), ref_data(0, 0, 0),
	tag(list, adjacent), ref_data(1, 0, 1),
	tag(list, link),     ref_data(2, 0, 2),
	tag(list, nil),      ref_data(3, 0, 0)
)}, from_buf[5], to_buf[6];

struct lxspace from, to;

union lxvalue root;

void before_each_test(void)
{
	memcpy(from_buf, state, sizeof state);
	init_allocspace(&from, from_buf, 5);
	init_tospace(&to, to_buf, 6);
	from.tag_free.cell = from_buf;
	root = lx_list(mklist(from_buf, 0));
}

int test_recursively_defined_structure_is_properly_copied(void)
{
	union lxvalue first, second, third, fourth;

	first = lx_compact(root, &from, &to);

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
