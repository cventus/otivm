#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "list.h"

static struct lxref reserve_tagged(struct lxmem *mem, size_t n)
{
	size_t new_cells, new_offset, new_spans;
	size_t new_cell_total;
	size_t tag_cells_max, mark_cells, span_cells;

	if (n <= mem->space.tag_free.offset) {
		/* allocate from current span */
		mem->space.tag_free.offset -= n;
		return mem->space.tag_free;
	} else {
		/* allocate the rest from the current span and more from new
		   ones */
		new_cells = n - mem->space.tag_free.offset;
		mem->space.tag_free.offset = 0;
	}

	/* Ensure there's enough room for the new cells */
	new_cell_total = space_spans(&mem->space)*CELL_SPAN + new_cells;
	mark_cells = mark_cell_count(new_cell_total);
	span_cells = ceil_div(new_cell_total, CELL_SPAN) * SPAN_LENGTH;
	tag_cells_max = mem->space.end - mem->space.raw_free;
	if (tag_cells_max - mark_cells < span_cells) {
		longjmp(mem->escape, mem->oom);
	}

	/* Move free pointer */
	new_spans = ceil_div(new_cells, CELL_SPAN) * SPAN_LENGTH;
	new_offset = CELL_SPAN - (new_cells % CELL_SPAN);
	mem->space.tag_free.cell -= new_spans;
	mem->space.tag_free.offset = new_offset % CELL_SPAN;

	return mem->space.tag_free;
}

struct lxlist lx_cons(
	struct lxmem *mem,
	union lxvalue val,
	struct lxlist list)
{
	struct lxref res, cdr;
	enum cdr_code cc;

	cc = list.ref.cell ? cdr_link : cdr_nil;
	res = reserve_tagged(mem, cc == cdr_link ? 2 : 1);
	*ref_tag(res) = mktag(cc, val.tag);
	switch (val.tag) {
	default: abort();
	case lx_nil_tag: ref_data(res)->i = 0; break;
	case lx_list_tag: setref(ref_data(res), val.list.ref); break;
	case lx_bool_tag: ref_data(res)->i = val.b; break;
	case lx_int_tag: ref_data(res)->i = val.i; break;
#if lxfloat
	case lx_float_tag: ref_data(res)->f = val.f; break;
#endif
	}
	if (cc == cdr_link) {
		cdr = forward(res);
		*ref_tag(cdr) = mktag(cdr_nil, lx_list_tag);
		setref(ref_data(cdr), list.ref);
	}
	return (struct lxlist) {
		.ref.tag = lx_list_tag,
		.ref.offset = res.offset,
		.ref.cell = res.cell,
	};
}
