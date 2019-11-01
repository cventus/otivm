#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ok/ok.h"
#include "lx32x4.h"

enum symbols { LAMBDA, X, Y };

struct lxvalue
int_one,
int_zero,
#ifdef lxfloat
float_one,
float_zero,
#endif
bool_true,
bool_false,
list_a,
list_a_copy,
list_b,
list_b_copy,
list_c;

#include STATE_DEFINITION

void before_each_test(void)
{
	list_a = mklist(state + list_a_cell, list_a_offset).value;

	list_a_copy = mklist(state + list_a_copy_cell, list_a_copy_offset).value;

	list_b = mklist(state + list_b_cell, list_b_offset).value;
	list_b_copy = mklist(state + list_b_copy_cell, list_b_copy_offset).value;

	list_c = mklist(state + list_c_cell, list_c_offset).value;

	int_one = lx_valuei(1);
	int_zero = lx_valuei(0);
	float_one = lx_valuef(1);
	float_zero = lx_valuef(0);
	bool_true = lx_valueb(true);
	bool_false = lx_valueb(false);
}

int test_int_one_equals_self(void)
{
	return assert_eq(int_one, int_one);
}

int test_int_one_does_not_equal_zero(void)
{
	return assert_neq(int_one, int_zero);
}

#ifdef lxfloat
int test_float_one_equals_int_one(void)
{
	return assert_eq(float_one, int_one);
}

int test_float_one_equals_self(void)
{
	return assert_eq(float_one, float_one);
}

int test_float_one_does_not_equal_zero(void)
{
	return assert_neq(float_one, float_zero);
}
#endif

int test_true_equals_true(void)
{
	return assert_eq(bool_true, bool_true);
}

int test_false_equals_false(void)
{
	return assert_eq(bool_false, bool_false);
}

int test_true_does_not_equal_false(void)
{
	return assert_neq(bool_true, bool_false);
}

int test_list_equals_itself(void)
{
	return assert_eq(list_a, list_a);
}

int test_list_equals_copy_of_itself(void)
{
	return assert_eq(list_a, list_a_copy);
}

int test_deeply_nested_list_equals_copy_of_itself(void)
{
	return assert_eq(list_b, list_b_copy);
}

int test_different_lists_are_not_equal(void)
{
	return assert_neq(list_a, list_c);
}
