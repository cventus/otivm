#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>

#include "ok/ok.h"
#include "lx32x4.h"

union lxcell data[11];
struct lxmem mem;
struct lx_list list, list_tail;
lxtag const *t;

void before_each_test(void)
{
	mem.oom = OOM_COMPACT;
	init_cons(&mem.alloc, data, 11);
}

int test_cons_one_element(void)
{
	/* we should not run out of memory */
	if (setjmp(mem.escape)) { fail_test(0); }

	list = lx_cons(&mem, lx_int(42), lx_empty_list());

	assert_eq(lx_car(list), lx_int(42));
	assert_eq(lx_list(lx_cdr(list)), lx_list(lx_empty_list()));

	return ok;
}

int test_cons_should_make_two_subsequent_allocations_adjacent(void)
{
	/* we should not run out of memory */
	if (setjmp(mem.escape)) { fail_test(0); }

	list_tail = lx_cons(&mem, lx_int(2), lx_empty_list());
	list = lx_cons(&mem, lx_int(1), list_tail);

	/* assert logical structure */
	assert_eq(lx_car(list), lx_int(1));
	assert_eq(lx_car(list_tail), lx_int(2));
	assert_eq(lx_list(lx_cdr(list)), lx_list(list_tail));
	assert_eq(lx_list(lx_cdr(list_tail)), lx_list(lx_empty_list()));

	/* assert physical structure */
	assert_int_eq(mem.alloc.tag_free.offset, 2);

	t = list.ref.cell->t;
	assert_int_eq(t[2], mktag(2, lx_int_tag));
	assert_int_eq(t[3], mktag(1, lx_int_tag));

	return ok;
}

int test_cons_should_link_two_non_adjacent_allocations(void)
{
	/* we should not run out of memory */
	if (setjmp(mem.escape)) { fail_test(0); }

	list_tail = lx_cons(&mem, lx_int(2), lx_empty_list());

	/* garbage allocation makes list segments to be non-adjacent */
	lx_cons(&mem, lx_list(lx_empty_list()), lx_empty_list());

	list = lx_cons(&mem, lx_int(1), list_tail);

	/* assert logical structure */
	assert_eq(lx_car(list), lx_int(1));
	assert_eq(lx_car(list_tail), lx_int(2));
	assert_eq(lx_list(lx_cdr(list)), lx_list(list_tail));
	assert_eq(lx_list(lx_cdr(list_tail)), lx_list(lx_empty_list()));

	/* assert physical structure */
	assert_int_eq(mem.alloc.tag_free.offset, 0);

	t = list.ref.cell->t;
	assert_int_eq(t[0], mktag(0, lx_int_tag));
	assert_int_eq(t[1], mktag(1, lx_list_tag));
	assert_int_eq(t[2], mktag(1, lx_nil_tag));
	assert_int_eq(t[3], mktag(1, lx_int_tag));

	return ok;
}

int test_cons_calls_longjmp_when_it_runs_out_of_memory(void)
{
	int i;
	volatile int result;

	/* we should not run out of memory */
	if (setjmp(mem.escape)) {
		if (result) {
			fail_test("ran out of memory too soon\n");
		}
		return result;
	}

	/* eight singleton lists should fit */ 
	result = -1;
	for (i = 0; i < 8; i ++) {
		list = lx_cons(&mem, lx_int(0), lx_empty_list());
	}
	result = 0;
	lx_cons(&mem, lx_int(0), lx_empty_list());
	return -1;
}

int test_cons_five_elements(void)
{
	/* we should not run out of memory */
	if (setjmp(mem.escape)) { fail_test(0); }

	list = lx_cons(&mem, lx_int(4), lx_empty_list());
	list = lx_cons(&mem, lx_int(3), list);
	list = lx_cons(&mem, lx_int(2), list);
	list = lx_cons(&mem, lx_int(1), list);

	assert_eq(lx_car(list), lx_int(1));
	assert_eq(lx_car(lx_cdr(list)), lx_int(2));
	assert_eq(lx_car(lx_cdr(lx_cdr(list))), lx_int(3));
	assert_eq(lx_car(lx_cdr(lx_cdr(lx_cdr(list)))), lx_int(4));

	return ok;
}
