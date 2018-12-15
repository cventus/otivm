#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>

#include "base/mem.h"
#include "base/wbuf.h"
#include "ok/ok.h"
#include "adt/bheap.h"

static int intcmp(void const *a, void const *b)
{
	int lhs = *(int const *)a, rhs = *(int const *)b;
	if (lhs < rhs) return -1;
	else if (lhs > rhs) return 1;
	else return 0;
}

void expect_ints(int const *heap, int const *expected, size_t n)
{
	size_t i;
	int h, e;

	for (i = 0; i < n; i++) {
		h = heap[i];
		e = expected[i];
		if (h != e) {
			printf("got %d at %zd, expected %d\n", h, i, e);
			ok = -1;
		}
	}
}

int test_insert_a_new_minimum_integer(void)
{
	int const expected[3] = { 1, 3, 2 };
	int heap[3] = { 2, 3 };

	(void)bheap_insert((int[]){ 1 }, heap, 2, sizeof(int), intcmp);
	expect_ints(heap, expected, 3);
	return ok;
}

int test_minimum_item_added_at_index_0(void)
{
	int heap[10];
	size_t nmemb = 0;

	(void)bheap_insert((int[]){ 9 }, heap, nmemb++, sizeof(int), intcmp);
	(void)bheap_insert((int[]){ 5 }, heap, nmemb++, sizeof(int), intcmp);
	(void)bheap_insert((int[]){ 3 }, heap, nmemb++, sizeof(int), intcmp);
	(void)bheap_insert((int[]){ 4 }, heap, nmemb++, sizeof(int), intcmp);
	(void)bheap_insert((int[]){ 6 }, heap, nmemb++, sizeof(int), intcmp);

	if (heap[0] != 3) {
		printf("minimum element not found at heap[0]\n");
		ok = -1;
	}
	return ok;
}

int test_remove_minimum_element(void)
{
	int heap[] = {
		6,
		12,               48,
		18,      24,      60, 54,
		36, 30,  42
	};
	int expected[] = {
		12,
		18,               48,
		30,      24,      60, 54,
		36, 42
	};

	bheap_remove(heap, heap, length_of(heap), sizeof(int), intcmp);
	expect_ints(heap, expected, length_of(expected));
	return ok;
}

int test_remove_element_inside_heap(void)
{
	int heap[] = {
		6,
		12,               48,
		18,      24,      60, 54,
		36, 30,  42
	};
	int expected[] = {
		6,
		12,               42,
		18,      24,      48, 54,
		36, 30
	};

	int *sixty = heap + 5;
	bheap_remove(sixty, heap, length_of(heap), sizeof(int), intcmp);
	expect_ints(heap, expected, length_of(expected));
	return ok;
}

static void insert_ints(int const *src, size_t n, int *arr)
{
	size_t i;
	int const *p;

	for (i = 0, p = src; i < n; i++, p++) {
		(void)bheap_insert(p, arr, i, sizeof(int), intcmp);
	}
}

static int check_int_heap(int *heap, int const *exp, size_t n)
{
	size_t i, nmemb;
	int min;

	for (i = 0; i < n; i++) {
		min = heap[0];
		if (min != exp[i]) {
			printf("got %d at %zd, expected %d\n", min, i, exp[i]);
			ok = -1;
			return -1;
		}
		nmemb = n - i;
		bheap_remove(heap, heap, nmemb, sizeof(int), intcmp);
	}
	return 0;
}

int test_heapify_array_of_integers(void)
{
#define N length_of(exp)
	static int const exp[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	int heap[N] = { 1, 10, 3, 8, 7, 5, 2, 4, 9, 6, 0 };

	init_bheap(heap, N, sizeof(int), intcmp);
	return check_int_heap(heap, exp, N);
#undef N
}

int test_heapify_array_of_descending_integers(void)
{
#define N length_of(exp)
	static int const exp[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	int heap[N] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

	init_bheap(heap, N, sizeof(int), intcmp);
	return check_int_heap(heap, exp, N);
#undef N
}

int test_insert_integers_into_heap(void)
{
#define N length_of(exp)
	static int const exp[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	static int const ins[N] = { 1, 10, 3, 8, 7, 5, 2, 4, 9, 6, 0 };
	int heap[N];

	insert_ints(ins, length_of(ins), heap);
	return check_int_heap(heap, exp, N);
#undef N
	return ok;
}

int test_insert_descending_integers_into_heap(void)
{
#define N length_of(exp)
	static int const exp[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	static int const ins[N] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
	int heap[N];

	insert_ints(ins, length_of(ins), heap);
	return check_int_heap(heap, exp, N);
#undef N
}
