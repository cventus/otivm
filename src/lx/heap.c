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
#include "alloc.h"
#include "list.h"
#include "heap.h"

#define DEFAULT_SIZE 4*1024

/* minimum space size is one complete span, plus one word for mark bits */
#define MIN_SPACE_SIZE (sizeof (union lxcell) * (SPAN_LENGTH + 1))
#define MIN_SIZE (4 * MIN_SPACE_SIZE)

/* member_ptr describes a `union lxcell *` or `char *` pointer field within
   a `struct lxheap` object which point into the heap and needs to be updated
   if the heap is reallocated */
struct member_ptr {
	size_t offset;
	/* `char *` is found within `struct lxvalue` and `union cell *` are
	   used elsewhere. Since these types aren't compatible according to the
	   C standard, it's necessary to cast them appropriately before
	   accessing them */
	enum { CHAR, CELL } type;
};

static union lxcell *get_ptr_at(struct lxheap *, struct member_ptr);
static void set_ptr_at(struct lxheap *, struct member_ptr, union lxcell *);

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

	/* round down to multiple of two cells */
	heap_size = heap_size & ~(size_t)(2*CELL_SIZE - 1);

	if (heap_size < MIN_SIZE) {
		heap_size = MIN_SIZE;
	}

	return heap_size;
}

static int realloc_heap(struct lxheap *heap, size_t adjusted_new_size)
{
	size_t total_cells, bitmap_cells, semispace_cells;
	void *new_heap;

	assert(adjusted_new_size % CELL_SIZE == 0);

	new_heap = realloc(heap->first, adjusted_new_size);
	if (!new_heap) {
		return -1;
	}
	heap->first = new_heap;
	heap->end = (union lxcell *)((char *)new_heap + adjusted_new_size);

	total_cells = adjusted_new_size / CELL_SIZE;
	/* N bits: N*total_cells = 2*N*semispace_cells + 2*semispace_cells */
	semispace_cells = total_cells * ((0.5 * LX_BITS) / (LX_BITS + 1.0));
	bitmap_cells = total_cells - 2*semispace_cells;

	heap->first = new_heap;
	heap->second = heap->first + semispace_cells;
	heap->bitset = heap->second + semispace_cells;
	heap->end = (union lxcell *)((char *)new_heap + adjusted_new_size);

	assert((size_t)(heap->end - heap->bitset) >= bitmap_cells);
	(void)bitmap_cells;

	return 0;
}

struct lxheap *lx_make_heap(size_t init_size, struct lx_config const *config)
{
	struct lxheap *heap;
	size_t heap_size, semispace_cells;

	heap = malloc(sizeof *heap);
	if (!heap) { return NULL; }

	heap->config = config ? *config : default_config;
	heap->first = NULL;
	heap_size = adjust_heap_size(&heap->config, init_size);
	if (realloc_heap(heap, heap_size)) {
		free(heap);
		return NULL;
	}
	heap->root = lx_empty_list().value;
	semispace_cells = heap->second - heap->first;
	init_cons(&heap->alloc, heap->first, semispace_cells);

	/* reserve space for sentinel value */
	heap->alloc.tag_free = backward(heap->alloc.tag_free);

	return heap;
}

void lx_free_heap(struct lxheap *heap)
{
	if (heap) {
		free(heap->first);
		free(heap);
	}
}

struct lxvalue lx_heap_value(struct lxheap const *heap)
{
	return heap->root;
}

size_t lx_heap_size(struct lxheap const *heap)
{
	return (unsigned char *)heap->end - (unsigned char *)heap->first;
}

static size_t semispace_size(struct lxheap const *heap)
{
	return (unsigned char *)heap->second - (unsigned char *)heap->first;
}

static union lxcell *get_ptr_at(struct lxheap *heap, struct member_ptr ptr)
{
	unsigned char *field;

	field = (unsigned char *)heap + ptr.offset;
	/* both branches likely generate the same code, but an implementations
	   might theoretically do some conversion when casting pointer typess */
	if (ptr.type == CHAR) {
		return (union lxcell *)*(char **)field;
	} else {
		return *(union lxcell **)field;
	}
}

static void set_ptr_at(
	struct lxheap *heap,
	struct member_ptr ptr,
	union lxcell *value)
{
	unsigned char *field;

	field = (unsigned char *)heap + ptr.offset;
	if (ptr.type == CHAR) {
		*(char **)field = (char *)value;
	} else {
		*(union lxcell **)field = value;
	}
}

static bool is_ref_value(struct lxvalue val)
{
	switch (val.tag) {
	case lx_list_tag: return !lx_is_empty_list(lx_list(val));
	case lx_tree_tag: return !lx_is_empty_tree(lx_tree(val));
	case lx_string_tag: return true;
	default: return false;
	}
}

/* grow heap - might or might not require garbage collection before
   allocating again */
int lx_resize_heap(struct lxheap *heap, size_t new_size)
{
	/* list of external pointers that need to be updated */
	static struct member_ptr const member_offsets[] = {
		{ offsetof(struct lxheap, alloc.max_addr), CELL },
		{ offsetof(struct lxheap, alloc.tag_free.s), CHAR },
		{ offsetof(struct lxheap, alloc.raw_free), CELL },
		{ offsetof(struct lxheap, root.s), CHAR },
	};
#define MEMBERS_MAX (sizeof member_offsets / sizeof member_offsets[0])

	size_t old_size, adjusted_new_size, i, max_member, old_semi, new_semi;
	ptrdiff_t save[MEMBERS_MAX], offset;
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
	if (!is_ref_value(heap->root)) {
		max_member--;
	}
	offset = *min_addr - heap->first;
	for (i = 0; i < max_member; i++) {
		save[i] = get_ptr_at(heap, member_offsets[i]) - *min_addr;
	}

	/* which space is currently used for allocation? */
	alloc_space = *min_addr < heap->second ? first_half : second_half;
	old_semi = semispace_size(heap);
	if (realloc_heap(heap, adjusted_new_size)) { return -1; }
	new_semi = semispace_size(heap);

	/* restore allocation pointers */
	if (alloc_space == second_half && new_semi < 2*old_semi) {
		/* shift data backward to start */
		memmove(heap->first, heap->first + offset, old_semi);
		*min_addr = heap->first;
	} else {
		*min_addr = heap->first + offset;
	}
	for (i = 0; i < max_member; i++) {
		set_ptr_at(heap, member_offsets[i], *min_addr + save[i]);
	}

	return 0;
}

/* swap raw_free and tag_free */
static inline void swap_allocation_pointers(struct lxalloc *alloc)
{
	union lxcell *cell;

	cell = alloc->raw_free;
	alloc->raw_free = ref_cell(alloc->tag_free);
	if (alloc->tag_free.offset) {
		alloc->raw_free += 1 + alloc->tag_free.offset;
	}
	alloc->tag_free = mkref(0, 0, cell);
}

/* semi-space garbage collection */
int lx_gc(struct lxheap *heap)
{
	union lxcell *from, *to;
	size_t semispace_cells, bitset_size;

	semispace_cells = heap->second - heap->first;
	from = heap->alloc.min_addr;
	to = from < heap->second ? heap->second : heap->first;
	init_tospace(&heap->alloc, to, semispace_cells);
	bitset_size = sizeof(*heap->bitset)*(heap->end - heap->bitset);
	heap->root = lx_compact(
		heap->root,
		from,
		&heap->alloc,
		heap->bitset,
		bitset_size);
	swap_allocation_pointers(&heap->alloc);

	return 0;
}
