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

static struct lxvalue
list_integers(struct lxstate *s, struct lxvalue root, void *param)
{
	int n, i;
	struct lxlist list;

	(void)root;
	n = *(int const *)param;
	list = lx_empty_list();
	i = n;
	while (i --> 0) {
		list = lx_cons(s, lx_valuei(i), list);
	}
	return list.value;
}

static struct lxvalue
set_nil(struct lxstate *s, struct lxvalue root, void *param)
{
	(void)s;
	(void)root;
	(void)param;
	return lx_empty_list().value;
}

static struct lxvalue
push_integer(struct lxstate *s, struct lxvalue root, void *param)
{
	int n;
	struct lxlist list;

	n = *(int *)param;
	list = lx_list(root);

	return lx_cons(s, lx_valuei(n), list).value;
}

static struct lxvalue
pop_integers(struct lxstate *s, struct lxvalue root, void *param)
{
	int n;
	struct lxlist list;

	(void)s;
	list = lx_list(root);
	n = *(int *)param;

	return lx_drop(list, n).value;
}

static struct lxvalue
sprintf_an_integer(struct lxstate *s, struct lxvalue root, void *param)
{
	(void)root;
	return lx_sprintf(s, "string-%d", *(int const *)param).value;
}

static struct lxvalue
create_association_list(struct lxstate *s, struct lxvalue root, void *param)
{
	struct lxlist list, elem;

#define nil lx_empty_list()
#define cons(a, b) lx_cons(s, (a), (b))
#define pair(a, b) cons(a, cons(b, nil))

	(void)param;
	(void)root;

	list = nil;

	elem = pair(lx_strdup(s, "c").value, lx_valuei(3));
	list = cons(elem.value, list);

	elem = pair(lx_strdup(s, "b").value, lx_valuei(2));
	list = cons(elem.value, list);

	elem = pair(lx_strdup(s, "a").value, lx_valuei(1));
	list = cons(elem.value, list);

#undef cons
#undef nil

	return list.value;
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
	assert_tag_eq(result.value.tag, lx_list_tag);
	assert_list_eq(lx_list(result.value), lx_empty_list());

	return 0;
}

int test_basic_list_modification(void)
{
	struct lxresult result;
	struct lxvalue root;

	heap = lx_make_heap(0, NULL);

	result = lx_modify(heap, list_integers, (int []){ 2 });
	if (result.status) {
		fail_test("lx_modify returned: %d\n", result.status);
	}
	root = result.value;

	assert_tag_eq(root.tag, lx_list_tag);
	assert_eq(lx_nth(lx_list(root), 0), lx_valuei(0));
	assert_eq(lx_nth(lx_list(root), 1), lx_valuei(1));

	return 0;
}

int test_modify_should_garbage_collect(void)
{
	int i, j;
	struct lxresult result;
	struct lxvalue root;

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
	struct lxvalue root;
	struct lxlist list;

	heap = lx_make_heap(4096, NULL);

	result = lx_modify(heap, set_nil, NULL);
	assert_status_eq(result.status, 0);

	for (i = 1; i <= 10000; i++) {
		k = rand() % i;
		result = lx_modify(heap, list_integers, &k);
		assert_status_eq(result.status, 0);
		root = lx_heap_value(heap);
		list = lx_list(root);
		for (j = 0; j < k; j++) {
			assert_eq(lx_car(list), lx_valuei(j));
			list = lx_cdr(list);
		}
		assert_list_eq(list, lx_empty_list());
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
	assert_int_eq(lx_strlen(ref_to_string(result.value)), (sizeof "string-42") - 1);

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
	assert_int_eq(lx_strlen(ref_to_string(result.value)), (sizeof "string-100") - 1);

	return 0;
}

int test_allocate_and_garbage_collect_a_string(void)
{
	struct lxvalue root;

	heap = lx_make_heap(0, NULL);
	lx_modify(heap, sprintf_an_integer, (int[]){ 42 });
	lx_gc(heap);
	root = lx_heap_value(heap);

	assert_tag_eq(root.tag, lx_string_tag);
	assert_str_eq(root.s, "string-42");
	assert_int_eq(lx_strlen(ref_to_string(root)), (sizeof "string-42") - 1);

	return 0;
}

int test_allocate_lists_and_strings(void)
{
	struct lxvalue root;

	heap = lx_make_heap(0, NULL);
	lx_modify(heap, create_association_list, (int[]){ 42 });
	root = lx_heap_value(heap);

	assert_str_eq(lx_nth(lx_list(lx_nth(lx_list(root), 0)), 0).s, "a");
	assert_int_eq(lx_nth(lx_list(lx_nth(lx_list(root), 0)), 1).i, 1);

	assert_str_eq(lx_nth(lx_list(lx_nth(lx_list(root), 1)), 0).s, "b");
	assert_int_eq(lx_nth(lx_list(lx_nth(lx_list(root), 1)), 1).i, 2);

	assert_str_eq(lx_nth(lx_list(lx_nth(lx_list(root), 2)), 0).s, "c");
	assert_int_eq(lx_nth(lx_list(lx_nth(lx_list(root), 2)), 1).i, 3);

	return 0;
}

int test_garbage_collect_lists_and_strings(void)
{
	struct lxvalue root;

	heap = lx_make_heap(0, NULL);
	lx_modify(heap, create_association_list, (int[]){ 42 });
	lx_gc(heap);
	root = lx_heap_value(heap);

	assert_str_eq(lx_nth(lx_list(lx_nth(lx_list(root), 0)), 0).s, "a");
	assert_int_eq(lx_nth(lx_list(lx_nth(lx_list(root), 0)), 1).i, 1);

	assert_str_eq(lx_nth(lx_list(lx_nth(lx_list(root), 1)), 0).s, "b");
	assert_int_eq(lx_nth(lx_list(lx_nth(lx_list(root), 1)), 1).i, 2);

	assert_str_eq(lx_nth(lx_list(lx_nth(lx_list(root), 2)), 0).s, "c");
	assert_int_eq(lx_nth(lx_list(lx_nth(lx_list(root), 2)), 1).i, 3);

	return 0;
}
