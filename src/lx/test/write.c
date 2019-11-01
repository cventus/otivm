#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#include "ok/ok.h"
#include "lx32x4.h"

static struct lxheap *heap;
static char const *string;

void before_each_test(void)
{
	heap = lx_make_heap(0, NULL);
}

void after_each_test(void)
{
	lx_free_heap(heap);
}

static struct lxvalue write_it(struct lxmem *mem, struct lxvalue val, va_list ap)
{
	struct lxstring str;
	struct lxread result;
	char const *p;

	(void)val;
	p = va_arg(ap, char const *);

	result = lx_read(mem, p);
	str = lx_write(mem, result.value);
	string = str.value.s;

	return str.value;
}

static void do_write(char const *str)
{
	string = lx_modifyl(heap, write_it, str).value.s;
}

int test_read_true(void)
{
	do_write("#t");
	assert_str_eq(string, "#t");
	return 0;
}

int test_read_false(void)
{
	do_write("#f");
	assert_str_eq(string, "#f");
	return 0;
}

int test_write_int_0(void)
{
	do_write("0");
	assert_str_eq(string, "0");
	return 0;
}

int test_write_int_1(void)
{
	do_write("1");
	assert_str_eq(string, "1");
	return 0;
}

int test_write_int_minus_1(void)
{
	do_write("-1");
	assert_str_eq(string, "-1");
	return 0;
}

int test_write_int_max(void)
{
	char int_min[80];
	lxint max;

	max = JOIN(INT, JOIN(LX_BITS, _MAX));
	int_min[sizeof int_min - 1] = '\0';
	snprintf(int_min, sizeof int_min - 1, "%" JOIN(PRId, LX_BITS), max);

	do_write(int_min);
	assert_str_eq(string, int_min);
	return 0;
}

int test_write_int_min(void)
{
	char int_min[80];
	lxint min;

	min = JOIN(INT, JOIN(LX_BITS, _MIN));
	int_min[sizeof int_min - 1] = '\0';
	snprintf(int_min, sizeof int_min - 1, "%" JOIN(PRId, LX_BITS), min);

	do_write(int_min);
	assert_str_eq(string, int_min);
	return 0;
}

int test_write_symbol_string(void)
{
	do_write("hello");
	assert_str_eq(string, "hello");
	return 0;
}

int test_write_string_with_space(void)
{
	do_write("\"hello, world\"");
	assert_str_eq(string, "\"hello, world\"");
	return 0;
}

int test_write_string_with_sharp_sign(void)
{
	do_write("\"foo#bar\"");
	assert_str_eq(string, "\"foo#bar\"");
	return 0;
}

int test_write_string_with_escaped_char(void)
{
	do_write("\"foo\\Abar\"");
	assert_str_eq(string, "fooAbar");
	return 0;
}

int test_write_string_with_escaped_backslash(void)
{
	do_write("\"foo\\\\bar\"");
	assert_str_eq(string, "\"foo\\\\bar\"");
	return 0;
}

int test_write_string_with_quote(void)
{
	do_write("\"foo\\\"bar\"");
	assert_str_eq(string, "\"foo\\\"bar\"");
	return 0;
}

int test_write_string_with_parenthesis(void)
{
	do_write("\"foo(bar\"");
	assert_str_eq(string, "\"foo(bar\"");
	return 0;
}

int test_write_lists(void)
{
	char const *expr = "((lambda (x) (x x)) (lambda (y) (y y)))";
	do_write(expr);
	assert_str_eq(string, expr);
	return 0;
}

int test_write_trees(void)
{
	char const *expr = "(one () {(a {}) (b (string list)) (c {(x y)})})";
	do_write(expr);
	assert_str_eq(string, expr);
	return 0;
}
