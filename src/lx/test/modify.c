#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

#include "ok/ok.h"
#include "lx32x4.h"

static union lxvalue
list_integers(struct lxmem *mem, union lxvalue root, void *param)
{
	int n, i;
	struct lxlist list;

	(void)root;
	n = *(int const *)param;
	list = lx_empty_list();
	i = n;
	while (i --> 0) {
		list = lx_cons(mem, lx_int(i), list);
	}
	return lx_list(list);
}

static union lxvalue
set_nil(struct lxmem *mem, union lxvalue root, void *param)
{
	(void)mem;
	(void)root;
	(void)param;
	return lx_list(lx_empty_list());
}

static union lxvalue
push_integer(struct lxmem *mem, union lxvalue root, void *param)
{
	int n;
	struct lxlist list;

	n = *(int *)param;
	list = root.list;

	return lx_list(lx_cons(mem, lx_int(n), list));
}

static union lxvalue
pop_integers(struct lxmem *mem, union lxvalue root, void *param)
{
	int n;
	struct lxlist list;

	(void)mem;
	list = root.list;
	n = *(int *)param;

	return lx_list(lx_drop(list, n));
}

static union lxvalue
sprintf_an_integer(struct lxmem *mem, union lxvalue root, void *param)
{
	(void)root;
	return lx_sprintf(mem, "string-%d", *(int const *)param);
}

static union lxvalue
create_association_list(struct lxmem *mem, union lxvalue root, void *param)
{
	struct lxlist list, elem;

#define nil lx_empty_list()
#define cons(a, b) lx_cons(mem, (a), (b))
#define pair(a, b) cons(a, cons(b, nil))

	(void)param;
	(void)root;

	list = nil;

	elem = pair(lx_strdup(mem, "c"), lx_int(3));
	list = cons(lx_list(elem), list);

	elem = pair(lx_strdup(mem, "b"), lx_int(2));
	list = cons(lx_list(elem), list);

	elem = pair(lx_strdup(mem, "a"), lx_int(1));
	list = cons(lx_list(elem), list);

#undef cons
#undef nil

	return lx_list(list);
}

static struct lxheap *heap = NULL;

void after_each_test(void)
{
	lx_free_heap(heap);
}

int test_modify_should_set_nil(void)
{
	struct lxresult result;

	heap = lx_make_heap(0, NULL);

	result = lx_modify(heap, set_nil, NULL);
	assert_status_eq(result.status, 0);
	assert_tag_eq(result.value.tag, lx_nil_tag);

	return 0;
}

int test_basic_list_modification(void)
{
	struct lxresult result;
	union lxvalue root;

	heap = lx_make_heap(0, NULL);

	result = lx_modify(heap, list_integers, (int []){ 2 });
	if (result.status) {
		fail_test("lx_modify returned: %d\n", result.status);
	}
	root = result.value;

	assert_tag_eq(root.tag, lx_list_tag);
	assert_eq(lx_nth(root.list, 0), lx_int(0));
	assert_eq(lx_nth(root.list, 1), lx_int(1));

	return 0;
}

int test_modify_should_garbage_collect(void)
{
	int i, j;
	struct lxresult result;
	union lxvalue root;

	/* room for four compact list cells (+ tag cell, mark bits) */
	heap = lx_make_heap(sizeof (union lxcell) * 6 * 2, NULL);

	result = lx_modify(heap, set_nil, NULL);
	assert_status_eq(result.status, 0);

	j = 0;

	/* allocate 3 */
	for (i = 0; i < 3; i++) {
		result = lx_modify(heap, push_integer, &j);
		assert_status_eq(result.status, 0);
		j++;
	}

	/* pop two */
	result = lx_modify(heap, pop_integers, (int[]){ 2 });
	assert_status_eq(result.status, 0);

	/* allocate 3 more */
	for (i = 0; i < 3; i++) {
		result = lx_modify(heap, push_integer, &j);
		assert_status_eq(result.status, 0);
		j++;
	}

	root = lx_heap_value(heap);
	assert_serialize_eq(root, "(5 4 3 0)");

	return 0;
}

int test_modify_should_garbage_collect_many_times(void)
{
	int i, j, k;
	struct lxresult result;
	union lxvalue root;
	struct lxlist list;

	heap = lx_make_heap(4096, NULL);

	result = lx_modify(heap, set_nil, NULL);
	assert_status_eq(result.status, 0);

	for (i = 1; i <= 10000; i++) {
		k = rand() % i;
		result = lx_modify(heap, list_integers, &k);
		assert_status_eq(result.status, 0);
		root = lx_heap_value(heap);
		list = root.list;
		for (j = 0; j < k; j++) {
			assert_tag_eq(list.tag, lx_list_tag);
			assert_eq(lx_car(list), lx_int(j));
			list = lx_cdr(list);
		}
		assert_tag_eq(list.tag, lx_nil_tag);
	}

	return 0;
}

int test_allocate_a_string(void)
{
	struct lxresult result;

	heap = lx_make_heap(0, NULL);
	result = lx_modify(heap, sprintf_an_integer, (int[]){ 42 });

	assert_status_eq(result.status, 0);
	assert_tag_eq(result.value.tag, lx_string_tag);
	assert_str_eq(result.value.s, "string-42");
	assert_int_eq(lx_strlen(result.value), (sizeof "string-42") - 1);

	return 0;
}

int test_allocate_100_strings(void)
{
	struct lxresult result;
	int i;

	heap = lx_make_heap(0, NULL);
	for (i = 1; i <= 100; i++) {
		result = lx_modify(heap, sprintf_an_integer, &i);
		assert_status_eq(result.status, 0);
		assert_tag_eq(result.value.tag, lx_string_tag);
	}
	assert_str_eq(result.value.s, "string-100");
	assert_int_eq(lx_strlen(result.value), (sizeof "string-100") - 1);

	return 0;
}

int test_allocate_and_garbage_collect_a_string(void)
{
	union lxvalue root;

	heap = lx_make_heap(0, NULL);
	lx_modify(heap, sprintf_an_integer, (int[]){ 42 });
	lx_gc(heap);
	root = lx_heap_value(heap);

	assert_tag_eq(root.tag, lx_string_tag);
	assert_str_eq(root.s, "string-42");
	assert_int_eq(lx_strlen(root), (sizeof "string-42") - 1);

	return 0;
}

int test_allocate_lists_and_strings(void)
{
	union lxvalue root;

	heap = lx_make_heap(0, NULL);
	lx_modify(heap, create_association_list, (int[]){ 42 });
	root = lx_heap_value(heap);

	assert_str_eq(lx_nth(lx_nth(root.list, 0).list, 0).s, "a");
	assert_int_eq(lx_nth(lx_nth(root.list, 0).list, 1).i, 1);

	assert_str_eq(lx_nth(lx_nth(root.list, 1).list, 0).s, "b");
	assert_int_eq(lx_nth(lx_nth(root.list, 1).list, 1).i, 2);

	assert_str_eq(lx_nth(lx_nth(root.list, 2).list, 0).s, "c");
	assert_int_eq(lx_nth(lx_nth(root.list, 2).list, 1).i, 3);

	return 0;
}

int test_garbage_collect_lists_and_strings(void)
{
	union lxvalue root;

	heap = lx_make_heap(0, NULL);
	lx_modify(heap, create_association_list, (int[]){ 42 });
	lx_gc(heap);
	root = lx_heap_value(heap);

	assert_str_eq(lx_nth(lx_nth(root.list, 0).list, 0).s, "a");
	assert_int_eq(lx_nth(lx_nth(root.list, 0).list, 1).i, 1);

	assert_str_eq(lx_nth(lx_nth(root.list, 1).list, 0).s, "b");
	assert_int_eq(lx_nth(lx_nth(root.list, 1).list, 1).i, 2);

	assert_str_eq(lx_nth(lx_nth(root.list, 2).list, 0).s, "c");
	assert_int_eq(lx_nth(lx_nth(root.list, 2).list, 1).i, 3);

	return 0;
}
