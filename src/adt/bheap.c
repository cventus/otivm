#include <assert.h>
#include <string.h>

#include "include/bheap.h"

/* return index of the parent (non-root) node `i` */
static size_t parent(size_t i)
{
	return (i - 1) >> 1;
}

/* return index of the left child of node `i` */
static size_t left(size_t i)
{
	return (i << 1) + 1;
}

/* return index of the right child of node `i` */
static size_t right(size_t i)
{
	return (i + 1) << 1;
}

/* return node index in the heap of node pointer `elem` */
static size_t index_of(void *heap, void *elem, size_t size)
{
	return (size_t)(((char *)elem - (char *)heap) / size);
}

/* return pointer to node `i` of the heap with the given element size */
static void *get_node(void *heap, size_t i, size_t size)
{
	return (char *)heap + (i*size);
}

/* in-place swap of non-overlapping memory ranges */
static void memswap(void *a, void *b, size_t size)
{
	unsigned char *restrict p, *restrict q;
	size_t i;

	p = a;
	q = b;
	for (i = 0; i < size; i++) {
		unsigned char t = p[i];
		p[i] = q[i];
		q[i] = t;
	}
}

/* if initially node `i` is empty and available, successively copy parent
   contents to child and return the now vacant ancestor node index where
   `key` can be stored, maintaining the heap invariants */
static size_t bubble(
	size_t i,
	void const *key,
	void *heap,
	size_t size,
	int (*compar)(void const *, void const *))
{
	size_t j, k;
	void *p, *dest;

	dest = get_node(heap, i, size);
	for (j = i; j > 0; j = k) {
		k = parent(j);
		p = get_node(heap, k, size);
		if (compar(p, key) < 0) { break; }
		(void)memcpy(dest, p, size);
		dest = p;
	}
	return j;
}

/* return child node index that the free slot at node `i` should swap with so
   that the value that `key` points to could be inserted while retaining the
   heap invariants, or `i` if no swaps are needed */
static size_t sink_to(
	size_t i,
	void const *key,
	void *heap,
	size_t nmemb,
	size_t size,
	int (*compar)(void const *, void const *))
{
	size_t l, r, min;
	void *p;
	void const *q;
	
	min = i;
	q = key;

	l = left(i);
	if (l < nmemb) {
		p = get_node(heap, l, size);
		if (compar(p, q) < 0) {
			min = l;
			q = p;
		}
	}
	r = right(i);
	if (r < nmemb) {
		p = get_node(heap, r, size);
		if (compar(p, q) < 0) { min = r; }
	}
	return min;
}

void init_bheap(
	void *heap,
	size_t nmemb,
	size_t size,
	int (*compar)(void const *, void const *))
{
	size_t i, j, k;
	void *src, *p;

	/* start from the first node with at least one child */
	k = nmemb / 2;
	while (k --> 0) {
		i = k;
		src = get_node(heap, i, size);
		/* move node down the tree while it's bigger than a child */
		while (j = sink_to(i, src, heap, nmemb, size, compar), j != i) {
			/* swap elements in-place */
			p = get_node(heap, j, size);
			memswap(src, p, size);
			i = j;
			src = p;
		}
	}
}

void *bheap_insert(
	void const *src,
	void *heap,
	size_t nmemb,
	size_t size,
	int (*compar)(void const *, void const *))
{
	void *dest;

	if (src == NULL) { return NULL; }

	assert(heap != 0);
	assert(size > 0);
	assert(compar != 0);

	/* initially add to bottom and bubble up from there */
	dest = get_node(heap, bubble(nmemb, src, heap, size, compar), size);
	return memcpy(dest, src, size);
}

void bheap_remove(
	void *elem,
	void *heap,
	size_t nmemb,
	size_t size,
	int (*compar)(void const *, void const *))
{
	size_t i, j;
	void *dest, *src, *p;

	if (elem == 0) { return; } /* null not found */
	if (nmemb < 2) { return; } /* one is no-op, empty heap allowed */

	assert(heap != 0);
	assert(size > 0);
	assert(compar != 0);

	/* move last item in heap to removed node's location */
	src = get_node(heap, nmemb - 1, size);

	if (elem == heap) {
		/* typical case of remove-min */
		i = 0;
		dest = heap;
	} else {
		/* move towards root as far as possible like an insert */
		i = index_of(heap, elem, size);
		i = bubble(i, src, heap, size, compar);
		dest = get_node(heap, i, size);
	}
	/* move down towards leafs to restore heap invariants */
	while (j = sink_to(i, src, heap, nmemb, size, compar), j != i) {
		/* copy child value to vacant node */
		p = get_node(heap, j, size);
		(void)memcpy(dest, p, size);
		dest = p;
		i = j;
	}
	(void)memcpy(dest, src, size);
}
