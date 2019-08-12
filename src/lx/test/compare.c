#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ok/ok.h"
#include "lx32x4.h"

union lxvalue
int_one,
int_zero,
float_one,
float_zero,
bool_true,
bool_false,
str_hello_world,
str_goodbye_world,
list_0,
list_1223,
list_1227,
list_123,
list_1234,
list_2,
tree_1,
tree_3,
tree_4,
tree_1234_a,
tree_1234_b,
tree_123,
tree_124;

#include STATE_DEFINITION

void before_each_test(void)
{
	int_one = lx_int(1);
	int_zero = lx_int(0);
	float_one = lx_float(1);
	float_zero = lx_float(0);
	bool_true = lx_bool(true);
	bool_false = lx_bool(false);

	str_hello_world = mkstr(state + str_hello_world_cell);
	str_goodbye_world = mkstr(state + str_goodbye_world_cell);

	list_0 = lx_list(mklist(state + list_0_cell, list_0_offset));
	list_1223 = lx_list(mklist(state + list_1223_cell, list_1223_offset));
	list_1227 = lx_list(mklist(state + list_1227_cell, list_1227_offset));
	list_123 = lx_list(mklist(state + list_123_cell, list_123_offset));
	list_1234 = lx_list(mklist(state + list_1234_cell, list_1234_offset));
	list_2 = lx_list(mklist(state + list_2_cell, list_2_offset));

	tree_1 = lx_tree(mktree(state + tree_1_cell, tree_1_offset));
	tree_3 = lx_tree(mktree(state + tree_3_cell, tree_3_offset));
	tree_4 = lx_tree(mktree(state + tree_4_cell, tree_4_offset));
	tree_1234_a = lx_tree(mktree(state + tree_1234_a_cell, tree_1234_a_offset));
	tree_1234_b = lx_tree(mktree(state + tree_1234_b_cell, tree_1234_b_offset));
	tree_123 = lx_tree(mktree(state + tree_123_cell, tree_123_offset));
	tree_124 = lx_tree(mktree(state + tree_124_cell, tree_124_offset));
}

#define assert_order(lt, gt) \
	assert_int_sign(lx_compare(lt, gt), -1); \
	assert_int_sign(lx_compare(gt, lt), 1);

#define assert_same(a, b) \
	assert_int_eq(lx_compare(a, b), 0); \
	assert_int_eq(lx_compare(b, a), 0); \

int test_compare_booleans(void)
{
	assert_order(bool_false, bool_true);
	assert_same(bool_false, bool_false);
	assert_same(bool_true, bool_true);
	return 0;
}

int test_compare_integers(void)
{
	assert_order(int_zero, int_one);
	assert_same(int_one, int_one);
	return 0;
}

int test_compare_floats(void)
{
	assert_order(float_zero, float_one);
	assert_same(float_one, float_one);
	return 0;
}

int test_compare_strings_lexicographically(void)
{
	assert_order(str_goodbye_world, str_hello_world);
	return 0;
}

int test_compare_lists_lexicographically(void)
{
	assert_order(list_0, list_1223);
	assert_order(list_0, list_1227);
	assert_order(list_0, list_123);
	assert_order(list_0, list_1234);
	assert_order(list_0, list_2);

	assert_order(list_1223, list_1227);
	assert_order(list_1223, list_123);
	assert_order(list_1223, list_1234);
	assert_order(list_1223, list_2);

	assert_order(list_1227, list_123);
	assert_order(list_1227, list_1234);
	assert_order(list_1227, list_2);

	assert_order(list_123, list_1234);
	assert_order(list_123, list_2);

	assert_order(list_1234, list_2);

	return 0;
}

int test_compare_trees_lexicographically(void)
{
	assert_order(tree_1, tree_123);
	assert_order(tree_1, tree_1234_a);
	assert_order(tree_1, tree_1234_b);
	assert_order(tree_1, tree_124);
	assert_order(tree_1, tree_3);
	assert_order(tree_1, tree_4);

	assert_order(tree_123, tree_1234_a);
	assert_order(tree_123, tree_1234_b);
	assert_order(tree_123, tree_124);
	assert_order(tree_123, tree_3);
	assert_order(tree_123, tree_4);

	// different structure but logically the same
	assert_same(tree_1234_a, tree_1234_b);

	assert_order(tree_1234_a, tree_124);
	assert_order(tree_1234_a, tree_3);
	assert_order(tree_1234_a, tree_4);

	assert_order(tree_124, tree_3);
	assert_order(tree_124, tree_4);

	assert_order(tree_3, tree_4);

	return 0;
}
