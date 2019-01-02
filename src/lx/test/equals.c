#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "ok/ok.h"
#include "lx32x4.h"

#define listval(s, n) \
	((union lxvalue) { .list = mklist(memory + s*SPAN_LENGTH, n) })

enum symbols { LAMBDA, X, Y };

union lxvalue
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

union lxcell const memory[] = {
   span(/* 0 */
	tag(int, nil),       int_data(X),
	tag(int, link),      int_data(X),
	cdr_tag,             ref_data(2, 0, 0),
	tag(int, adjacent),  int_data(LAMBDA) /* list_a */
), span(/* 1 */
	tag(list, adjacent), ref_data(0, -1, 0),
	tag(list, nil),      ref_data(1, -1, 1),
	tag(int, adjacent),  int_data(X),
	tag(int, nil),       int_data(X)
), span(/* 2 */
	tag(list, nil),      ref_data(0, -1, 2),
	tag(list, link),     ref_data(1, -1, 3),
	cdr_tag,             ref_data(2, 0, 0),
	tag(int, link),      int_data(LAMBDA) /* list_a_copy */
), span(/* 3 */
	cdr_tag,             ref_data(0, -1, 1),
	tag(list, adjacent), ref_data(1, -3, 3), /* list_b */
	tag(list, nil),      ref_data(2, -1, 3),
	tag(list, adjacent), ref_data(3, -1, 3) /* list_b_copy */
), span(/* 4 */
	tag(list, nil),      ref_data(0, -4, 3),
	tag(int, adjacent),  int_data(X),
	tag(int, nil),       int_data(Y),
	tag(int, adjacent),  int_data(LAMBDA) /* list_c */
), span(/* 5 */
	tag(list, adjacent), ref_data(0, -4, 3),
	tag(list, nil),      ref_data(1, -1, 1),
	tag(int, nil),       int_data(0xDEAD),
	tag(int, nil),       int_data(0xDEAD)
)
};

void before_each_test(void)
{
	list_a = listval(0, 3);
	list_a_copy = listval(2, 3);

	list_b = listval(3, 1);
	list_b_copy = listval(3, 3);

	list_c = listval(4, 3);

	int_one = lx_int(1);
	int_zero = lx_int(0);
#ifdef lxfloat
	float_one = lx_float(1);
	float_zero = lx_float(0);
#endif
	bool_true = lx_bool(true);
	bool_false = lx_bool(false);
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
