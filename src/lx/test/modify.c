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

int test_modify_should_set_nil(void)
{
	struct lxheap *heap;
	struct lxresult result;

	heap = lx_make_heap(0, NULL);

	result = lx_modify(heap, set_nil, NULL);
	assert_status_eq(result.status, 0);
	assert_tag_eq(result.value.tag, lx_nil_tag);

	lx_free_heap(heap);
	return 0;
}

int test_basic_list_modification(void)
{
	struct lxheap *heap;
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

	lx_free_heap(heap);
	return 0;
}

int test_modify_should_garbage_collect(void)
{
	int i, j;
	struct lxheap *heap;
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

	lx_free_heap(heap);
	return 0;
}

int test_modify_should_garbage_collect_many_times(void)
{
	int i, j, k;
	struct lxheap *heap;
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

	lx_free_heap(heap);
	return 0;
}