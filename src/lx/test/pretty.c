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

static struct lxvalue write_it(struct lxstate *s, struct lxvalue val, va_list ap)
{
	struct lxstring str;
	struct lxread result;
	char const *p;

	(void)val;
	p = va_arg(ap, char const *);

	result = lx_read(s, p);
	str = lx_write_pretty(s, result.value);

	return str.value;
}

static void do_write(char const *str)
{
	struct lxresult result;

	result = lx_modifyl(heap, write_it, str);
	assert_int_eq(result.status, 0);
	string = result.value.s;
}

int test_short_lists_should_be_on_one_line(void)
{
	char const *expr = "((hello world) (foo bar))";
	do_write(expr);
	assert_str_eq(string, expr);
	return 0;
}

int test_long_lists_should_break_into_several_lines(void)
{
	char const *expr = "(one two (three four) five six seven eight)";
	char const *expected =
		"(one\n"
		" two\n"
		" (three four)\n"
		" five\n"
		" six\n"
		" seven\n"
		" eight)";
	do_write(expr);
	assert_str_eq(string, expected);
	return 0;
}

int test_single_element_trees_are_on_one_line(void)
{
	char const *expr = "{(foo bar)}";
	do_write(expr);
	assert_str_eq(string, expr);
	return 0;
}

int test_each_tree_entry_should_go_on_their_own_line(void)
{
	char const *expr = "{(first line) (second line)}";
	char const *expected =
		"{(first line)\n"
		" (second line)}";
	do_write(expr);
	assert_str_eq(string, expected);
	return 0;
}

int test_nested_structure_accumulate_indentation(void)
{
	char const *expr = "{(key-1 {(one 1) (two 2) (three 3)}) (key-2) (key-3 \"long list\" with a lot of string elements)}";
	char const *expected =
		"{(key-1\n"
		"  {(one 1)\n"
		"   (three 3)\n"
		"   (two 2)})\n"
		" (key-2)\n"
		" (key-3\n"
		"  \"long list\"\n"
		"  with\n"
		"  a\n"
		"  lot\n"
		"  of\n"
		"  string\n"
		"  elements)}";
	do_write(expr);
	assert_str_eq(string, expected);
	return 0;
}
