#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>
#include <limits.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "alloc.h"
#include "str.h"
#include "list.h"

int lx_reserve_tagged(struct lxalloc *alloc, size_t n, struct lxvalue *ref)
{
	size_t new_cells, new_span_cells, new_offset;

	if (n <= alloc->tag_free.offset) {
		/* allocate from current span */
		alloc->tag_free.offset -= n;
		*ref = alloc->tag_free;
		return 0;
	} else {
		/* allocate the rest from the current span and more from new
		   ones */
		new_cells = n - alloc->tag_free.offset;
	}

	/* Ensure there's enough room for the new cells */
	new_span_cells = ceil_div(new_cells, CELL_SPAN) * SPAN_LENGTH;
	if (alloc_free_count(alloc) < new_span_cells) {
		return -1;
	}

	/* Move free pointer */
	new_offset = (CELL_SPAN - (new_cells % CELL_SPAN)) % CELL_SPAN;
	*ref = alloc->tag_free = mkref(
		alloc->tag_free.tag,
		new_offset,
		ref_cell(alloc->tag_free) - new_span_cells);

	return 0;
}

void lx_set_cell_data(union lxcell *data, struct lxvalue val)
{
	switch (val.tag) {
	default: abort();
	case lx_list_tag:
	case lx_tree_tag:
	case lx_string_tag:
		setref(data, val);
		break;
	case lx_bool_tag: data->i = val.b; break;
	case lx_int_tag: data->i = val.i; break;
	case lx_float_tag: data->f = val.f; break;
	}
}
