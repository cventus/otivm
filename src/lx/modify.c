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

jmp_buf *lx_prepare_start(struct lxstate *state, struct lxheap *heap)
{
	state->heap = heap;
	state->alloc = heap->alloc;
	state->iterations = 0;
	state->status = lx_state_ok;
	return &state->restart;
}

/* commit new allocations */
void lx_end(struct lxstate *state, struct lxvalue value)
{
	state->status = lx_state_ok;
	state->heap->root = value;
	state->heap->alloc = state->alloc;
}

void lx_handle_out_of_memory(struct lxstate *state)
{
	enum { restart_block = 1, abort_block = -1 };

	if (state->status == lx_state_heap_size) {
		/* unhandled lx_start block - avoid infinite loop and abort */
		abort();
	}
	if (state->iterations > 0) {
		if (lx_resize_heap(state->heap, lx_heap_size(state->heap)*2)) {
			state->status = lx_state_heap_size;
			longjmp(state->restart, abort_block);
		}
	}

	lx_gc(state->heap);
	state->iterations++;
	state->alloc = state->heap->alloc;
	longjmp(state->restart, restart_block);
}
