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

void before_each_test(void)
{
	heap = lx_make_heap(0, NULL);
}

void after_each_test(void)
{
	lx_free_heap(heap);
}

static union lxvalue read_it(struct lxmem *mem, union lxvalue val, va_list ap)
{
	char const *str;
	struct lxread *r;

	(void)val;
	r = va_arg(ap, struct lxread *);
	str = va_arg(ap, char const *);

	*r = lx_read(mem, str);
	return r->value;
}

static void do_read(char const *str, enum lx_tag expected)
{
	lx_modifyl(heap, read_it, &result, str);
	assert_int_eq(result.err, LX_READ_OK);
	assert_tag_eq(result.value.tag, expected);
}

int test_read_true(void)
{
	do_read("#t", lx_bool_tag);
	assert_eq(result.value, lx_bool(1));
	return 0;
}

int test_read_false(void)
{
	do_read("#f", lx_bool_tag);
	assert_eq(result.value, lx_bool(0));
	return 0;
}

int test_read_int_1(void)
{
	do_read("1", lx_int_tag);
	assert_eq(result.value, lx_int(1));
	return 0;
}

int test_read_int_minus_1(void)
{
	do_read("-1", lx_int_tag);
	assert_eq(result.value, lx_int(-1));
	return 0;
}

int test_read_int_max(void)
{
	char int_max[80];
	lxint max;

	max = JOIN(INT, JOIN(LX_BITS, _MAX));
	int_max[sizeof int_max - 1] = '\0';
	snprintf(int_max, sizeof int_max - 1, "%" JOIN(PRId, LX_BITS), max);

	do_read(int_max, lx_int_tag);
	assert_eq(result.value, lx_int(max));
	return 0;
}

int test_read_int_min(void)
{
	char int_min[80];
	lxint min;

	min = JOIN(INT, JOIN(LX_BITS, _MIN));
	int_min[sizeof int_min - 1] = '\0';
	snprintf(int_min, sizeof int_min - 1, "%" JOIN(PRId, LX_BITS), min);

	do_read(int_min, lx_int_tag);
	assert_eq(result.value, lx_int(min));
	return 0;
}

int test_read_string(void)
{
	do_read("\"hello, world\"", lx_string_tag);
	assert_str_eq(result.value.s, "hello, world");
	return 0;
}

int test_read_one_token(void)
{
	do_read("hello, world", lx_string_tag);
	assert_str_eq(result.value.s, "hello,");
	assert_str_eq(result.where, " world");
	return 0;
}

int test_read_empty_list(void)
{
	do_read("()", lx_list_tag);
	assert_eq(result.value, lx_list(lx_empty_list()));
	return 0;
}

int test_read_list(void)
{
	struct lxlist l;

	do_read("(a b c)", lx_list_tag);
	l = result.value.list;
	assert_int_eq(lx_length(l), 3);
	assert_str_eq(lx_nth(l, 0).s, "a");
	assert_str_eq(lx_nth(l, 1).s, "b");
	assert_str_eq(lx_nth(l, 2).s, "c");
	return 0;
}

int test_read_empty_tree(void)
{
	do_read("{}", lx_tree_tag);
	assert_eq(result.value, lx_tree(lx_empty_tree()));
	return 0;
}

int test_read_tree(void)
{
	struct lxtree t;
	struct lxlist l;

	do_read("{(1 a) (2 b) (3 c)}", lx_tree_tag);
	t = result.value.tree;
	assert_int_eq(lx_tree_size(t), 3);

	l = lx_tree_nth(t, 0);
	assert_int_eq(lx_length(l), 2);
	assert_eq(lx_nth(l, 0), lx_int(1));
	assert_str_eq(lx_nth(l, 1).s, "a");

	l = lx_tree_nth(t, 1);
	assert_int_eq(lx_length(l), 2);
	assert_eq(lx_nth(l, 0), lx_int(2));
	assert_str_eq(lx_nth(l, 1).s, "b");

	l = lx_tree_nth(t, 2);
	assert_int_eq(lx_length(l), 2);
	assert_eq(lx_nth(l, 0), lx_int(3));
	assert_str_eq(lx_nth(l, 1).s, "c");

	return 0;
}
