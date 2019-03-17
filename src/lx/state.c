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
	space_cells = heap_size / 2*CELL_SIZE;
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
