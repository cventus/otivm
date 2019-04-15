#include <stdarg.h>
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

union lxvalue lx_heap_value(struct lxheap const *heap)
{
	return heap->root;
}

size_t lx_heap_size(struct lxheap const *heap)
{
	return (unsigned char *)heap->end -(unsigned char *)heap->begin;
}

static union lxcell **ptr_at(struct lxheap *heap, size_t offset)
{
	return (union lxcell **)((unsigned char *)heap + offset);
}

/* grow heap - might or might not require garbage collection before
   allocating again */
int lx_resize_heap(struct lxheap *heap, size_t new_size)
{
	static size_t const member_offsets[] = {
		offsetof(struct lxheap, alloc.max_addr),
		offsetof(struct lxheap, alloc.tag_free.cell),
		offsetof(struct lxheap, alloc.raw_free),
		offsetof(struct lxheap, alloc.mark_bits),
		offsetof(struct lxheap, root.list.ref.cell),
	};
#define MEMBERS_MAX (sizeof member_offsets / sizeof member_offsets[0])

	size_t old_size, adjusted_new_size, i, max_member;
	ptrdiff_t save[MEMBERS_MAX], min_addr_offset;
	enum { first_half, second_half } alloc_space;
	union lxcell **min_addr;

	/* Shrinking heap not supported yet - do nothing. */
	old_size = lx_heap_size(heap);
	if (new_size < old_size) { return 0; }

	adjusted_new_size = adjust_heap_size(&heap->config, new_size);
	if (adjusted_new_size == old_size) { return 0; }

	/* save allocation pointers */
	min_addr = &heap->alloc.min_addr;
	max_member = MEMBERS_MAX;
	if (heap->root.tag != lx_list_tag) { max_member--; }
	min_addr_offset = *min_addr - heap->begin;
	for (i = 0; i < max_member; i++) {
		save[i] = *ptr_at(heap, member_offsets[i]) - *min_addr;
	}

	/* which space is currently used for allocation? */
	alloc_space = *min_addr < heap->mid ? first_half : second_half;
	if (realloc_heap(heap, adjusted_new_size)) { return -1; }

	/* restore allocation pointers */
	if (alloc_space == second_half && adjusted_new_size < 2*old_size) {
		/* shift data forward to new second half */
		memmove(heap->mid, heap->begin + min_addr_offset, old_size/2);
		*min_addr = heap->mid;
	} else {
		*min_addr = heap->begin + min_addr_offset;
	}
	for (i = 0; i < max_member; i++) {
		*ptr_at(heap, member_offsets[i]) = *min_addr + save[i];
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
	to = from < heap->mid ? heap->mid : heap->begin;
	init_tospace(&heap->alloc, to, semispace_cells);
	heap->root = lx_compact(heap->root, from, &heap->alloc);
	swap_allocation_pointers(&heap->alloc);

	return 0;
}
