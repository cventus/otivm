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

static int reserve_tagged(struct lxalloc *alloc, size_t n, struct lxref *ref)
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
	new_offset = (CELL_SPAN - (new_span_cells % CELL_SPAN)) % CELL_SPAN;
	alloc->tag_free.cell -= new_span_cells;
	alloc->tag_free.offset = new_offset;

	*ref = alloc->tag_free;
	return 0;
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
	if (reserve_tagged(&mem->alloc, cc == cdr_link ? 2 : 1, &res)) {
		longjmp(mem->escape, mem->oom);
	}
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
