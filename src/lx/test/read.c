#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#include "ok/ok.h"
#include "lx32x4.h"

struct lxheap *heap;
struct lxread result;
struct lxstate s[1];

void before_each_test(void)
{
	heap = lx_make_heap(0, NULL);
}

void after_each_test(void)
{
	lx_free_heap(heap);
}

static struct lxvalue do_read(char const *str, enum lx_tag expected)
{
	if (lx_start(s, heap) < 0) {
		fail_test("Memory ran out!\n");
	}

	result = lx_read(s, str);
	lx_end(s, result.value);

	assert_int_eq(result.status, LX_READ_OK);
	assert_tag_eq(result.value.tag, expected);
	assert_int_eq(s->status, lx_state_ok);

	return result.value;
}

int test_read_true(void)
{
	assert_eq(do_read("#t", lx_bool_tag), lx_valueb(1));
	return 0;
}

int test_read_false(void)
{
	assert_eq(do_read("#f", lx_bool_tag), lx_valueb(0));
	return 0;
}

int test_read_int_1(void)
{
	assert_eq(do_read("1", lx_int_tag), lx_valuei(1));
	return 0;
}

int test_read_int_minus_1(void)
{
	assert_eq(do_read("-1", lx_int_tag), lx_valuei(-1));
	return 0;
}

int test_read_int_max(void)
{
	char int_max[80];
	lxint max;

	max = JOIN(INT, JOIN(LX_BITS, _MAX));
	int_max[sizeof int_max - 1] = '\0';
	snprintf(int_max, sizeof int_max - 1, "%" JOIN(PRId, LX_BITS), max);

	assert_eq(do_read(int_max, lx_int_tag), lx_valuei(max));
	return 0;
}

int test_read_int_min(void)
{
	char int_min[80];
	lxint min;

	min = JOIN(INT, JOIN(LX_BITS, _MIN));
	int_min[sizeof int_min - 1] = '\0';
	snprintf(int_min, sizeof int_min - 1, "%" JOIN(PRId, LX_BITS), min);

	assert_eq(do_read(int_min, lx_int_tag), lx_valuei(min));
	return 0;
}

int test_read_string(void)
{
	char const *p = do_read("\"hello, world\"", lx_string_tag).s;
	assert_str_eq(p, "hello, world");
	return 0;
}

int test_read_one_token(void)
{
	char const *p = do_read("hello, world", lx_string_tag).s;
	assert_str_eq(p, "hello,");
	assert_str_eq(result.where, " world");
	return 0;
}

int test_read_nil(void)
{
	assert_eq(do_read("nil", lx_nil_tag), lx_nil());
	assert_eq(do_read(" nil ", lx_nil_tag), lx_nil());
	assert_eq(lx_car(lx_list(do_read("(nil)", lx_list_tag))), lx_nil());
	return 0;
}

int test_read_the_string_nil(void)
{
	char const *p = do_read("\"nil\"", lx_string_tag).s;
	assert_str_eq(p, "nil");
	return 0;
}

int test_read_empty_list(void)
{
	assert_eq(do_read("()", lx_list_tag), lx_empty_list().value);
	return 0;
}

int test_read_list(void)
{
	struct lxlist l = lx_list(do_read("(a b c)", lx_list_tag));
	assert_int_eq(lx_length(l), 3);
	assert_str_eq(lx_nth(l, 0).s, "a");
	assert_str_eq(lx_nth(l, 1).s, "b");
	assert_str_eq(lx_nth(l, 2).s, "c");
	return 0;
}

int test_read_empty_tree(void)
{
	assert_eq(do_read("{}", lx_tree_tag), lx_empty_tree().value);
	return 0;
}

int test_read_tree(void)
{
	struct lxtree t;
	struct lxlist l;

	t = lx_tree(do_read("{(1 a) (2 b) (3 c)}", lx_tree_tag));
	assert_int_eq(lx_tree_size(t), 3);

	l = lx_tree_nth(t, 0);
	assert_int_eq(lx_length(l), 2);
	assert_eq(lx_nth(l, 0), lx_valuei(1));
	assert_str_eq(lx_nth(l, 1).s, "a");

	l = lx_tree_nth(t, 1);
	assert_int_eq(lx_length(l), 2);
	assert_eq(lx_nth(l, 0), lx_valuei(2));
	assert_str_eq(lx_nth(l, 1).s, "b");

	l = lx_tree_nth(t, 2);
	assert_int_eq(lx_length(l), 2);
	assert_eq(lx_nth(l, 0), lx_valuei(3));
	assert_str_eq(lx_nth(l, 1).s, "c");

	return 0;
}

int test_heap_read_should_set_the_heaps_value(void)
{
	struct lxlist lst;

	result = lx_heap_read(heap, "(0 1 2)");
	assert_int_eq(result.status, LX_READ_OK);
	assert_tag_eq(result.value.tag, lx_list_tag);
	lst = lx_list(result.value);

	/* heap value is set */
	assert_eq(result.value, lx_heap_value(heap));

	/* value is list */
	assert_int_eq(lx_length(lst), 3);
	assert_eq(lx_nth(lst, 0), lx_valuei(0));
	assert_eq(lx_nth(lst, 1), lx_valuei(1));
	assert_eq(lx_nth(lst, 2), lx_valuei(2));

	return 0;
}

#define _stringify(x) #x
#define stringify(x) _stringify(x)

int test_heap_read_should_report_heap_size_limits(void)
{
	struct lxheap *small_heap;
	struct lx_config cfg = { 0 };

	static char const *big_expr = stringify(
	  (let ((digits "123456789")
	        (list ((quote a) (quote b) (quote c)))
	        (number 42))
	    (display digits)
	    (display list)
	    (display number))
	);

	small_heap = lx_make_heap(0, &cfg);
	result = lx_heap_read(small_heap, big_expr);
	assert_int_eq(result.status, LX_READ_HEAP_SIZE);

	lx_free_heap(small_heap);

	return 0;
}
