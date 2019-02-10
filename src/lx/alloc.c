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
	size_t m, span_cells, cells_required, empty_cells, mark_cells;

	if (n <= mem->space.tag_free.offset) {
		/* allocate from current span */
		mem->space.tag_free.offset -= n;
		return mem->space.tag_free;
	} else {
		/* allocate the rest from the current span and more */
		m = n - mem->space.tag_free.offset;
		mem->space.tag_free.offset = 0;
	}

	cells_required = m + ceil_div(m, CELL_SPAN);
	span_cells = ceil_div(cells_required, CELL_SPAN + 1) * (CELL_SPAN + 1);
	empty_cells = mem->space.tag_free.cell - mem->space.raw_free;
	mark_cells = mark_cell_count(cell_count(&mem->space) + cells_required);

	if (empty_cells - mark_cells < span_cells) {
		longjmp(mem->escape, mem->oom);
	}
	mem->space.tag_free.cell -= span_cells;
	mem->space.tag_free.offset = (CELL_SPAN - (m % CELL_SPAN)) % CELL_SPAN;

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
