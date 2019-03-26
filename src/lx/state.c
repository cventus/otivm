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
#include "state.h"

#define MIN_SPACE_SIZE (sizeof (union lxcell) * (SPAN_LENGTH + 1))
#define MIN_SIZE (2 * MIN_SPACE_SIZE)

static struct lx_config const default_config = {
	.init_size = 4*1024,
	.max_size = SIZE_MAX
};

struct lxstate *lx_make(struct lx_config const *config)
{
	struct lxstate *state;
	size_t heap_size, space_cells;
	struct lx_config const *c;
	union lxcell *heap;

	state = malloc(sizeof *state);
	if (!state) { return NULL; }

	c = config ? config : &default_config;

	heap_size = c->init_size;
	heap_size = c->init_size & ~(size_t)(2*CELL_SIZE - 1);
	if (heap_size < MIN_SIZE) { heap_size = MIN_SIZE; }
	space_cells = heap_size / (2*CELL_SIZE);
	heap = malloc(heap_size);
	if (!heap) {
		free(state);
		return NULL;
	}

	state->root = lx_list(lx_empty_list());
	state->config.init_size = heap_size;
	state->config.max_size = c->max_size;
	state->heap = heap;
	state->size = heap_size;
	init_allocspace(&state->alloc, heap, space_cells);
	init_tospace(&state->spare, heap + space_cells, space_cells);

	return state;
}

void lx_free(struct lxstate *state)
{
	if (state) {
		free(state->heap);
		free(state);
	}
}

union lxvalue lx_root(struct lxstate const *state)
{
	return state->root;
}

/* grow heap - might or might not require garbage collection before
   allocating again */
int lx_resize_heap(struct lxstate *state, size_t new_size)
{
	size_t space_cells, spare_offset;
	ptrdiff_t root_offset, tag_free_offset, raw_free_offset;
	union lxcell *new_heap;
	enum { alloc_spare, spare_alloc } order;

	if (new_size > state->config.max_size) {
		new_size = state->config.max_size;
	}
	if (new_size < state->size) {
		/* Shrinking heap not supported yet - do nothing. */
		return 0;
	}
	if (state->root.tag == lx_list_tag) {
		root_offset = state->root.list.ref.cell - state->heap;
	}
	/* only allocation-space need to be kept track of */
	tag_free_offset = state->alloc.tag_free.cell - state->heap;
	raw_free_offset = state->alloc.raw_free - state->heap;

	/* which space comes first? */
	order = state->alloc.begin == state->heap ? alloc_spare : spare_alloc;

	new_heap = realloc(state->heap, new_size);
	if (!new_heap) { return -1; }
	space_cells = new_size / (2*CELL_SIZE);
	state->heap = new_heap;
	state->size = new_size;
	if (state->root.tag == lx_list_tag) {
		state->root.list.ref.cell = new_heap + root_offset;
	}

	/* adjust extents of semi-spaces */
	if (order == alloc_spare) {
		state->alloc.begin = new_heap + 0;
		state->alloc.end = new_heap + space_cells;
		spare_offset = space_cells;
	} else {
		state->alloc.begin = new_heap + space_cells;
		state->alloc.end = new_heap + 2*space_cells;
		spare_offset = 0;
	}

	/* restore allocation space free pointers */
	state->alloc.raw_free = new_heap + raw_free_offset;
	state->alloc.tag_free.cell = new_heap + tag_free_offset;

	/* re-initialize spare space */
	init_tospace(&state->spare, new_heap + spare_offset, space_cells);

	return 0;
}

/* Semi-space garbage collection */
int lx_gc(struct lxstate *state)
{
	struct lxspace tmp;

	state->root = lx_compact(state->root, &state->alloc, &state->spare);

	set_tospace(&state->alloc);
	tmp = state->alloc;
	state->alloc = state->spare;
	state->spare = tmp;
	tospace_to_allocspace(&state->alloc);

	return 0;
}
