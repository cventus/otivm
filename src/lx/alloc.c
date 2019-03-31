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
	size_t new_cells, new_span_cells, new_offset;

	if (n <= mem->alloc.tag_free.offset) {
		/* allocate from current span */
		mem->alloc.tag_free.offset -= n;
		return mem->alloc.tag_free;
	} else {
		/* allocate the rest from the current span and more from new
		   ones */
		new_cells = n - mem->alloc.tag_free.offset;
		mem->alloc.tag_free.offset = 0;
	}

	/* Ensure there's enough room for the new cells */
	new_span_cells = ceil_div(new_cells, CELL_SPAN) * SPAN_LENGTH;
	if (alloc_free_count(&mem->alloc) < new_span_cells) {
		longjmp(mem->escape, mem->oom);
	}

	/* Move free pointer */
	new_offset = (CELL_SPAN - (new_span_cells % CELL_SPAN)) % CELL_SPAN;
	mem->alloc.tag_free.cell -= new_span_cells;
	mem->alloc.tag_free.offset = new_offset;

	return mem->alloc.tag_free;
}

struct lxlist lx_cons(
	struct lxmem *mem,
	union lxvalue val,
	struct lxlist list)
{
	struct lxref res, cdr;
	enum cdr_code cc;

	if (lx_is_empty_list(list)) {
		cc = cdr_nil;
	} else if (ref_eq(list.ref, mem->alloc.tag_free)) {
		cc = cdr_adjacent;
	} else {
		cc = cdr_link;
	}
	res = reserve_tagged(mem, cc == cdr_link ? 2 : 1);
	*ref_tag(res) = mktag(cc, val.tag);
	switch (val.tag) {
	case lx_nil_tag: ref_data(res)->i = 0; break;
	case lx_list_tag: setref(ref_data(res), val.list.ref); break;
	case lx_bool_tag: ref_data(res)->i = val.b; break;
	case lx_int_tag: ref_data(res)->i = val.i; break;
#if lxfloat
	case lx_float_tag: ref_data(res)->f = val.f; break;
#endif
	default: abort();
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
