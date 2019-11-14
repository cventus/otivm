#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include <signal.h>

#include "ok/ok.h"
#include "lx32x4.h"

static struct lxheap *heap = NULL;
static struct lxstate s[1];

static jmp_buf return_from_signal;

static void on_sigabrt(int sig)
{
	(void)sig;
	longjmp(return_from_signal, 1);
}

void after_each_test(void)
{
	lx_free_heap(heap);
}

int test_modify_should_set_nil(void)
{
	heap = lx_make_heap(0, NULL);

	lx_start(s, heap);
	lx_end(s, lx_empty_list().value);

	assert_eq(lx_heap_value(heap), lx_empty_list().value);
	return 0;
}

int test_start_should_return_negative_on_heap_exhaustion(void)
{
	struct lx_config cfg = { .max_size = 1 };
	struct lxlist lst;
	int i;

	heap = lx_make_heap(0, &cfg);
	if (lx_start(s, heap) < 0) {
		/* ok! */
		assert_int_eq(s->status, lx_state_heap_size);
		return 0;
	}
	lst = lx_empty_list();
	for (i = 0; i < 10000; i++) {
		lst = lx_cons(s, lx_valuei(42), lst);
	}
	lx_end(s, lx_empty_list().value);
	fail_test("Managed to allocate too much\n");

	return -1;
}

int test_modify_should_abort_if_heap_exhaustion_is_unhandled(void)
{
	struct lx_config cfg = { .max_size = 1 };
	struct lxlist lst;
	int i;

	if (setjmp(return_from_signal)) {
		return 0;
	}
	signal(SIGABRT, on_sigabrt);

	heap = lx_make_heap(0, &cfg);
	lx_start(s, heap);
	lst = lx_empty_list();
	for (i = 0; i < 10000; i++) {
		lst = lx_cons(s, lx_valuei(42), lst);
	}
	lx_end(s, lx_empty_list().value);
	fail_test("Managed to allocate too much\n");

	return -1;
}

int test_basic_list_modification(void)
{
	struct lxlist list;
	int i;

	heap = lx_make_heap(0, NULL);

	lx_start(s, heap);
	list = lx_empty_list();
	i = 2;
	while (i --> 0) {
		list = lx_cons(s, lx_valuei(i), list);
	}
	lx_end(s, lx_empty_list().value);

	assert_tag_eq(list.value.tag, lx_list_tag);
	assert_eq(lx_nth(list, 0), lx_valuei(0));
	assert_eq(lx_nth(list, 1), lx_valuei(1));

	return 0;
}

int test_modify_should_garbage_collect(void)
{
	int i, j;
	struct lxvalue root;
	struct lxlist list;

	/* room for four compact list cells (+ tag cell, mark bits) */
	heap = lx_make_heap(sizeof (union lxcell) * 6 * 2, NULL);

	lx_start(s, heap);
	list = lx_empty_list();
	lx_end(s, list.value);

	j = 0;

	/* allocate 3 */
	for (i = 0; i < 3; i++) {
		lx_start(s, heap);
		list.value = lx_heap_value(heap);
		list = lx_cons(s, lx_valuei(j), list);
		lx_end(s, list.value);
		j++;
	}

	/* pop two */
	lx_start(s, heap);
	list.value = lx_heap_value(heap);
	list = lx_drop(list, 2);
	lx_end(s, list.value);

	/* allocate 3 more */
	for (i = 0; i < 3; i++) {
		lx_start(s, heap);
		list.value = lx_heap_value(heap);
		list = lx_cons(s, lx_valuei(j), list);
		lx_end(s, list.value);
		j++;
	}

	root = lx_heap_value(heap);
	assert_serialize_eq(root, "(5 4 3 0)");

	return 0;
}

int test_modify_should_garbage_collect_many_times(void)
{
	int i, j, k, n;
	struct lxlist list;

	heap = lx_make_heap(4096, NULL);

	lx_start(s, heap);
	lx_end(s, lx_empty_list().value);

	for (i = 1; i <= 10000; i++) {
		k = rand() % i;

		/* create (0 1 2 3 ... [k-1]) */
		lx_start(s, heap);
		list = lx_empty_list();
		n = k;
		while (n --> 0) {
			list = lx_cons(s, lx_valuei(n), list);
		}
		lx_end(s, list.value);

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
	struct lxstring string;

	heap = lx_make_heap(0, NULL);

	lx_start(s, heap);
	string = lx_sprintf(s, "string-%d", 42);

	assert_tag_eq(string.value.tag, lx_string_tag);
	assert_str_eq(string.value.s, "string-42");
	assert_int_eq(lx_strlen(string), (sizeof "string-42") - 1);

	return 0;
}

int test_allocate_100_strings(void)
{
	struct lxstring string;
	int i;

	heap = lx_make_heap(0, NULL);
	for (i = 1; i <= 100; i++) {
		lx_start(s, heap);
		string = lx_sprintf(s, "string-%d", i);
		lx_end(s, string.value);

		assert_tag_eq(string.value.tag, lx_string_tag);
	}
	assert_str_eq(string.value.s, "string-100");
	assert_int_eq(lx_strlen(string), (sizeof "string-100") - 1);

	return 0;
}

int test_allocate_and_garbage_collect_a_string(void)
{
	struct lxvalue root;
	struct lxstring string;

	heap = lx_make_heap(0, NULL);

	lx_start(s, heap);
	string = lx_sprintf(s, "string-%d", 42);
	lx_end(s, string.value);

	lx_gc(heap);
	root = lx_heap_value(heap);

	assert_tag_eq(root.tag, lx_string_tag);
	assert_str_eq(root.s, "string-42");
	assert_int_eq(lx_strlen(lx_string(root)), (sizeof "string-42") - 1);

	return 0;
}

int test_allocate_lists_and_strings(void)
{
	struct lxlist list, elem;

	heap = lx_make_heap(0, NULL);

	lx_start(s, heap);

	list = lx_empty_list();

	elem = lx_pair(s, lx_strdup(s, "c").value, lx_valuei(3));
	list = lx_cons(s, elem.value, list);

	elem = lx_pair(s, lx_strdup(s, "b").value, lx_valuei(2));
	list = lx_cons(s, elem.value, list);

	elem = lx_pair(s, lx_strdup(s, "a").value, lx_valuei(1));
	list = lx_cons(s, elem.value, list);

	lx_end(s, list.value);

	assert_str_eq(lx_nth(lx_list(lx_nth(list, 0)), 0).s, "a");
	assert_int_eq(lx_nth(lx_list(lx_nth(list, 0)), 1).i, 1);

	assert_str_eq(lx_nth(lx_list(lx_nth(list, 1)), 0).s, "b");
	assert_int_eq(lx_nth(lx_list(lx_nth(list, 1)), 1).i, 2);

	assert_str_eq(lx_nth(lx_list(lx_nth(list, 2)), 0).s, "c");
	assert_int_eq(lx_nth(lx_list(lx_nth(list, 2)), 1).i, 3);

	return 0;
}

int test_garbage_collect_lists_and_strings(void)
{
	struct lxlist list, elem;

	heap = lx_make_heap(0, NULL);

	lx_start(s, heap);

	list = lx_empty_list();

	elem = lx_pair(s, lx_strdup(s, "c").value, lx_valuei(3));
	list = lx_cons(s, elem.value, list);

	elem = lx_pair(s, lx_strdup(s, "b").value, lx_valuei(2));
	list = lx_cons(s, elem.value, list);

	elem = lx_pair(s, lx_strdup(s, "a").value, lx_valuei(1));
	list = lx_cons(s, elem.value, list);

	lx_end(s, list.value);

	lx_gc(heap);
	list.value = lx_heap_value(heap);

	assert_str_eq(lx_nth(lx_list(lx_nth(list, 0)), 0).s, "a");
	assert_int_eq(lx_nth(lx_list(lx_nth(list, 0)), 1).i, 1);

	assert_str_eq(lx_nth(lx_list(lx_nth(list, 1)), 0).s, "b");
	assert_int_eq(lx_nth(lx_list(lx_nth(list, 1)), 1).i, 2);

	assert_str_eq(lx_nth(lx_list(lx_nth(list, 2)), 0).s, "c");
	assert_int_eq(lx_nth(lx_list(lx_nth(list, 2)), 1).i, 3);

	return 0;
}
