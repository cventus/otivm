#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#include "ok/ok.h"
#include "lx32x4.h"


static struct lxheap *heap;
struct lxstate s[1];

void before_each_test(void)
{
	heap = lx_make_heap(0, NULL);
}

void after_each_test(void)
{
	lx_free_heap(heap);
}

static char const *do_write(char const *str)
{
	struct lxread result;
	struct lxvalue value;

	if (lx_start(s, heap) < 0) {
		fail_test("Memory ran out!\n");
	}
	result = lx_read(s, str);
	assert_int_eq(result.status, LX_READ_OK);
	value = lx_write(s, result.value).value;
	lx_end(s, value);

	assert_int_eq(s->status, lx_state_ok);
	assert_tag_eq(value.tag, lx_string_tag);

	return value.s;
}

int test_write_true(void)
{
	assert_str_eq(do_write("#t"), "#t");
	return 0;
}

int test_write_false(void)
{
	assert_str_eq(do_write("#f"), "#f");
	return 0;
}

int test_write_int_0(void)
{
	assert_str_eq(do_write("0"), "0");
	return 0;
}

int test_write_int_1(void)
{
	assert_str_eq(do_write("1"), "1");
	return 0;
}

int test_write_int_minus_1(void)
{
	assert_str_eq(do_write("-1"), "-1");
	return 0;
}

int test_write_int_max(void)
{
	char int_min[80];
	lxint max;

	max = JOIN(INT, JOIN(LX_BITS, _MAX));
	int_min[sizeof int_min - 1] = '\0';
	snprintf(int_min, sizeof int_min - 1, "%" JOIN(PRId, LX_BITS), max);

	assert_str_eq(do_write(int_min), int_min);
	return 0;
}

int test_write_int_min(void)
{
	char int_min[80];
	lxint min;

	min = JOIN(INT, JOIN(LX_BITS, _MIN));
	int_min[sizeof int_min - 1] = '\0';
	snprintf(int_min, sizeof int_min - 1, "%" JOIN(PRId, LX_BITS), min);

	assert_str_eq(do_write(int_min), int_min);
	return 0;
}

int test_write_symbol_string(void)
{
	assert_str_eq(do_write("hello"), "hello");
	return 0;
}

int test_write_string_with_space(void)
{
	assert_str_eq(do_write("\"hello, world\""), "\"hello, world\"");
	return 0;
}

int test_write_string_with_sharp_sign(void)
{
	assert_str_eq(do_write("\"foo#bar\""), "\"foo#bar\"");
	return 0;
}

int test_write_string_with_escaped_char(void)
{
	assert_str_eq(do_write("\"foo\\Abar\""), "fooAbar");
	return 0;
}

int test_write_string_with_escaped_backslash(void)
{
	assert_str_eq(do_write("\"foo\\\\bar\""), "\"foo\\\\bar\"");
	return 0;
}

int test_write_string_with_quote(void)
{
	assert_str_eq(do_write("\"foo\\\"bar\""), "\"foo\\\"bar\"");
	return 0;
}

int test_write_string_with_parenthesis(void)
{
	assert_str_eq(do_write("\"foo(bar\""), "\"foo(bar\"");
	return 0;
}

int test_write_lists(void)
{
	char const *expr = "((lambda (x) (x x)) (lambda (y) (y y)))";
	assert_str_eq(do_write(expr), expr);
	return 0;
}

int test_write_trees(void)
{
	char const *expr = "(one () {(a {}) (b (string list)) (c {(x y)})})";
	assert_str_eq(do_write(expr), expr);
	return 0;
}

int test_write_nil(void)
{
	assert_str_eq(do_write("nil"), "nil");
	return 0;
}

int test_write_string_containing_nil(void)
{
	assert_str_eq(do_write("\"nil\""), "\"nil\"");
	return 0;
}
