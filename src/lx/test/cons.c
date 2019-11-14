#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>

#include "ok/ok.h"
#include "lx32x4.h"

union lxcell data[11];
struct lxstate s;
struct lx_list list, list_tail;
lxtag const *t;
static bool should_run_out_of_memory;
static jmp_buf on_oom;

void lx_handle_out_of_memory(struct lxstate *state)
{
	(void)state;

	if (should_run_out_of_memory) {
		longjmp(on_oom, 1);
	} else {
		fail_test("unexpectedly ran out of memory\n");
		abort();
	}
}

void before_each_test(void)
{
	init_cons(&s.alloc, data, length_of(data));
	should_run_out_of_memory = false;
}

int test_reserve_tagged(void)
{
	union lxcell cells[20];
	struct lxalloc alloc;
	struct lxvalue ref, expected;
	int i;

	init_cons(&alloc, cells, length_of(cells));
	expected = alloc.tag_free;

	for (i = 1; i <= CELL_SIZE*2; i++) {
		lx_reserve_tagged(&alloc, i, &ref);
		expected = backward(expected);
		assert_ref_eq(expected, ref);
		init_cons(&alloc, cells, length_of(cells));
	}

	return 0;
}

int test_cons_one_element(void)
{
	list = lx_cons(&s, lx_valuei(42), lx_empty_list());

	assert_eq(lx_car(list), lx_valuei(42));
	assert_eq(lx_cdr(list).value, lx_empty_list().value);

	return ok;
}

int test_cons_should_make_two_subsequent_allocations_adjacent(void)
{
	list_tail = lx_cons(&s, lx_valuei(2), lx_empty_list());
	list = lx_cons(&s, lx_valuei(1), list_tail);

	/* assert logical structure */
	assert_eq(lx_car(list), lx_valuei(1));
	assert_eq(lx_car(list_tail), lx_valuei(2));
	assert_eq(lx_cdr(list).value, list_tail.value);
	assert_eq(lx_cdr(list_tail).value, lx_empty_list().value);

	/* assert physical structure */
	assert_int_eq(s.alloc.tag_free.offset, 2);

	t = ref_cell(list.value)->t;
	assert_int_eq(t[2], mktag(2, lx_int_tag));
	assert_int_eq(t[3], mktag(1, lx_int_tag));

	return ok;
}

int test_cons_should_link_two_non_adjacent_allocations(void)
{
	list_tail = lx_cons(&s, lx_valuei(2), lx_empty_list());

	/* garbage allocation makes list segments to be non-adjacent */
	lx_cons(&s, lx_empty_list().value, lx_empty_list());

	list = lx_cons(&s, lx_valuei(1), list_tail);

	/* assert logical structure */
	assert_eq(lx_car(list), lx_valuei(1));
	assert_eq(lx_car(list_tail), lx_valuei(2));
	assert_eq(lx_cdr(list).value, list_tail.value);
	assert_eq(lx_cdr(list_tail).value, lx_empty_list().value);

	/* assert physical structure */
	assert_int_eq(s.alloc.tag_free.offset, 0);

	t = ref_cell(list.value)->t;
	assert_int_eq(t[0], mktag(0, lx_int_tag));
	assert_int_eq(t[1], mktag(1, lx_list_tag));
	assert_int_eq(t[2], mktag(1, lx_list_tag));
	assert_int_eq(t[3], mktag(1, lx_int_tag));

	return ok;
}

int test_cons_calls_longjmp_when_it_runs_out_of_memory(void)
{
	int i;

	should_run_out_of_memory = false;

	/* eight singleton lists should fit */ 
	for (i = 0; i < 8; i ++) {
		list = lx_cons(&s, lx_valuei(0), lx_empty_list());
	}
	if (setjmp(on_oom)) { return 0; }
	should_run_out_of_memory = true;
	lx_cons(&s, lx_valuei(0), lx_empty_list());
	return -1;
}

int test_cons_five_elements(void)
{
	list = lx_cons(&s, lx_valuei(4), lx_empty_list());
	list = lx_cons(&s, lx_valuei(3), list);
	list = lx_cons(&s, lx_valuei(2), list);
	list = lx_cons(&s, lx_valuei(1), list);

	assert_eq(lx_car(list), lx_valuei(1));
	assert_eq(lx_car(lx_cdr(list)), lx_valuei(2));
	assert_eq(lx_car(lx_cdr(lx_cdr(list))), lx_valuei(3));
	assert_eq(lx_car(lx_cdr(lx_cdr(lx_cdr(list)))), lx_valuei(4));

	return ok;
}
