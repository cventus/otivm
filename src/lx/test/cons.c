#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>

#include "ok/ok.h"
#include "lx32x4.h"

union lxcell data[10];
struct lxmem mem;
struct lx_list list;

void before_each_test(void)
{
	mem.oom = OOM_COMPACT;

	/* raw-free points to lowest address */
	mem.raw_free = data;

	/* tag-free points to highest address (multiple of CELL_SPAN+1) */
	mem.tag_free.offset = 0;
	mem.tag_free.cell = data + 10;
}

int test_cons_one_element(void)
{
	/* we should not run out of memory */
	if (setjmp(mem.escape)) return -1;

	list = lx_cons(&mem, lx_int(42), lx_empty_list());

	assert_eq(lx_car(list), lx_int(42));
	assert_eq(lx_list(lx_cdr(list)), lx_list(lx_empty_list()));

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
	if (setjmp(mem.escape)) return -1;

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
