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
	size_t m, spans, cells_required, free_space;

	if (n <= mem->tag_free.offset) {
		/* allocate from current span */
		mem->tag_free.offset -= n;
		return mem->tag_free;
	} else {
		/* allocate the rest from the current span and more */
		m = n - mem->tag_free.offset;
		mem->tag_free.offset = 0;
	}
	/* c = m + ceil(m/4) */
	cells_required = m + (m + CELL_SPAN - 1) / CELL_SPAN;

	/* s = ceil(c / 5) */
	spans = (cells_required + CELL_SPAN) / (CELL_SPAN + 1);

	free_space = mem->tag_free.cell - mem->raw_free;

	if (free_space < spans * (CELL_SPAN + 1)) {
		longjmp(mem->escape, mem->oom);
	}
	mem->tag_free.cell -= spans * (CELL_SPAN + 1);
	mem->tag_free.offset = (CELL_SPAN - (m % CELL_SPAN)) % CELL_SPAN;

	return mem->tag_free;
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
	case lx_list_tag: setref(ref_data(res), val.list.ref); break;
	case lx_bool_tag: ref_data(res)->i = val.b; break;
	case lx_int_tag: ref_data(res)->i = val.i; break;
#if lxfloat
	case lx_float_tag: ref_data(res)->f = val.f; break;
#endif
	}
	if (cc == cdr_link) {
		cdr = forward(res, lx_list_tag);
		*ref_tag(cdr) = mktag(cdr_nil, lx_list_tag);
		setref(ref_data(cdr), list.ref);
	}
	return (struct lxlist) {
		.ref.tag = lx_list_tag,
		.ref.offset = res.offset,
		.ref.cell = res.cell,
	};
}
