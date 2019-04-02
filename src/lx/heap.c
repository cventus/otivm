#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "list.h"
#include "heap.h"

#define DEFAULT_SIZE 4*1024

/* minimum space size is one complete span, plus one word for mark bits */
#define MIN_SPACE_SIZE (sizeof (union lxcell) * (SPAN_LENGTH + 1))
#define MIN_SIZE (4 * MIN_SPACE_SIZE)

static struct lx_config const default_config = {
	.max_size = SIZE_MAX,
};

static size_t adjust_heap_size(struct lx_config const *config, size_t new_size)
{
	size_t heap_size;

	heap_size = new_size;
	if (heap_size > config->max_size) {
		heap_size = config->max_size;
	}

	/* round to multiple of two cells */
	heap_size = heap_size & ~(size_t)(2*CELL_SIZE - 1);

	if (heap_size < MIN_SIZE) {
		heap_size = MIN_SIZE;
	}

	return heap_size;
}

static int realloc_heap(struct lxheap *heap, size_t adjusted_new_size)
{
	size_t semispace_cells;
	void *new_heap;

	new_heap = realloc(heap->begin, adjusted_new_size);
	if (!new_heap) {
		return -1;
	}
	semispace_cells = adjusted_new_size / (2*CELL_SIZE);
	heap->begin = new_heap;
	heap->mid = heap->begin + semispace_cells;
	heap->end = heap->mid + semispace_cells;

	assert((char *)heap->begin + adjusted_new_size == (char *)heap->end);

	return 0;
}

struct lxheap *lx_make_heap(size_t init_size, struct lx_config const *config)
{
	struct lxheap *heap;
	size_t heap_size, semispace_cells;

	heap = malloc(sizeof *heap);
	if (!heap) { return NULL; }

	heap->config = config ? *config : default_config;
	heap->begin = NULL;
	heap_size = adjust_heap_size(&heap->config, init_size);
	if (realloc_heap(heap, heap_size)) {
		free(heap);
		return NULL;
	}
	heap->root = lx_list(lx_empty_list());
	semispace_cells = heap->mid - heap->begin;
	init_cons(&heap->alloc, heap->begin, semispace_cells);

	return heap;
}

void lx_free_heap(struct lxheap *heap)
{
	if (heap) {
		free(heap->begin);
		free(heap);
	}
}

union lxvalue lx_heap_root(struct lxheap const *heap)
{
	return heap->root;
}

size_t lx_heap_size(struct lxheap const *heap)
{
	return heap->end - heap->begin;
}

static union lxcell **ptr_at(struct lxalloc *alloc, size_t offset)
{
	return (union lxcell **)((unsigned char *)alloc + offset);
}

/* grow heap - might or might not require garbage collection before
   allocating again */
int lx_resize_heap(struct lxheap *heap, size_t new_size)
{
	static size_t const member_offsets[] = {
		offsetof(struct lxalloc, tag_free.cell),
		offsetof(struct lxalloc, raw_free),
		offsetof(struct lxalloc, mark_bits),
	};
#define OFFSETS_MAX (sizeof member_offsets / sizeof member_offsets[0])

	struct lxalloc *alloc;
	size_t old_size, adjusted_new_size, i;
	ptrdiff_t save[OFFSETS_MAX], min_addr_off;
	enum { at_begin, at_mid } active_space;

	/* Shrinking heap not supported yet - do nothing. */
	old_size = lx_heap_size(heap);
	if (new_size < old_size) { return 0; }

	adjusted_new_size = adjust_heap_size(&heap->config, new_size);
	if (adjusted_new_size == old_size) { return 0; }

	alloc = &heap->alloc;

	/* save allocation pointers */
	min_addr_off = alloc->min_addr - heap->begin;
	for (i = 0; i < OFFSETS_MAX; i++) {
		save[i] = *ptr_at(alloc, member_offsets[i]) - alloc->min_addr;
	}

	/* which space is currently active? */
	active_space = alloc->min_addr == heap->begin ? at_begin : at_mid;

	if (realloc_heap(heap, adjusted_new_size)) { return -1; }

	/* restore allocation pointers */
	if (active_space == at_mid && adjusted_new_size < 2*old_size) {
		/* we allocated from the second semi-space [mid, end), and
		   it grew by less than double in size */
		alloc->min_addr = heap->mid;
		alloc->max_addr = heap->end;
		memmove(heap->mid, heap->begin + min_addr_off, old_size/2);
	} else {
		alloc->min_addr = heap->begin;
		alloc->max_addr = heap->mid;
	}
	for (i = 0; i < OFFSETS_MAX; i++) {
		*ptr_at(alloc, member_offsets[i]) = alloc->min_addr + save[i];
	}
	return 0;
}

/* semi-space garbage collection */
int lx_gc(struct lxheap *heap)
{
	union lxcell *from, *to;
	size_t semispace_cells;

	semispace_cells = heap->mid - heap->begin;
	from = heap->alloc.min_addr;
	to = from == heap->begin ? heap->mid : heap->begin;
	init_tospace(&heap->alloc, to, semispace_cells);
	heap->root = lx_compact(heap->root, from, &heap->alloc);
	swap_allocation_pointers(&heap->alloc);

	return 0;
}
