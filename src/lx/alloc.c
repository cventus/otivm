#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
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
#include "str.h"
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
	struct lxref ref, cdr;
	enum cdr_code cc;

	if (lx_is_empty_list(list)) {
		cc = cdr_nil;
	} else if (ref_eq(list.ref, mem->alloc.tag_free)) {
		cc = cdr_adjacent;
	} else {
		cc = cdr_link;
	}
	if (reserve_tagged(&mem->alloc, cc == cdr_link ? 2 : 1, &ref)) {
		longjmp(mem->escape, mem->oom);
	}
	*ref_tag(ref) = mktag(cc, val.tag);
	switch (val.tag) {
	case lx_nil_tag: ref_data(ref)->i = 0; break;
	case lx_list_tag: setref(ref_data(ref), val.list.ref); break;
	case lx_string_tag: setref(ref_data(ref), string_to_ref(val)); break;
	case lx_bool_tag: ref_data(ref)->i = val.b; break;
	case lx_int_tag: ref_data(ref)->i = val.i; break;
	case lx_float_tag:
#if lxfloat
		ref_data(ref)->f = val.f; break;
#endif
	default: abort();
	}
	if (cc == cdr_link) {
		cdr = forward(ref);
		*ref_tag(cdr) = mktag(cdr_nil, lx_list_tag);
		setref(ref_data(cdr), list.ref);
	}
	ref.tag = lx_list_tag;
	return ref_to_list(ref);
}

size_t lx_strlen(union lxvalue string)
{
	if (string.tag != lx_string_tag || string.s == NULL) {
		return 0;
	} else {
		return string_to_ref(string).cell->i;
	}
}

/* create a string of exactly n bytes (which are not null) from src */
static union lxvalue strdup_exact(struct lxmem *mem, char const *src, size_t n)
{
	size_t free_bytes;
	char *dest;
	union lxcell *sz;

	/* check available space */
	free_bytes = alloc_free_count(&mem->alloc) * sizeof (union lxcell);
	if (n + sizeof (union lxcell) >= free_bytes) {
		longjmp(mem->escape, mem->oom);
	}

	/* allocate string */
	sz = mem->alloc.raw_free;
	dest = (char *)(mem->alloc.raw_free + 1);
	mem->alloc.raw_free += ceil_div(n + 1, sizeof (union lxcell)) + 1;

	/* copy and nul-terminate */
	memcpy(dest, src, n);
	dest[n] = 0;
	sz->i = n;

	return (union lxvalue) {
		.tag = lx_string_tag,
		.s = dest,
	};
}

union lxvalue lx_strdup(struct lxmem *mem, char const *src)
{
	return strdup_exact(mem, src, strlen(src));
}

union lxvalue lx_strndup(struct lxmem *mem, char const *src, size_t n)
{
	char const *p;

	/* look for early NUL-terminator */
	p = memchr(src, 0, n);
	if (p) {
		return strdup_exact(mem, src, p - src);
	} else {
		return strdup_exact(mem, src, n);
	}
}

union lxvalue lx_vsprintf(struct lxmem *mem, char const *fmt, va_list ap)
{
	size_t cells, free_bytes;
	union lxcell *sz = mem->alloc.raw_free;
	char *dest = NULL;
	int n;

	cells = alloc_free_count(&mem->alloc);
	if (cells > 0) {
		dest = (char *)(mem->alloc.raw_free + 1);
		/* reserve one cell for length */
		cells--;
	}
	free_bytes = cells * sizeof (union lxcell);
	n = vsnprintf(dest, free_bytes, fmt, ap);
	if (n < 0) {
		abort(); // FIXME
	} else if ((size_t)n >= free_bytes) {
		// FIXME: set resize hint based on "n"?
		longjmp(mem->escape, mem->oom);
	}
	sz->i = n;

	mem->alloc.raw_free += ceil_div(n + 1, sizeof (union lxcell)) + 1;

	return (union lxvalue) {
		.tag = lx_string_tag,
		.s = dest,
	};
}

union lxvalue lx_sprintf(struct lxmem *mem, char const *fmt, ...)
{
	va_list ap;
	union lxvalue result;

	va_start(ap, fmt);
	result = lx_vsprintf(mem, fmt, ap);
	va_end(ap);

	return result;
}
