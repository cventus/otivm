/* 32-bit cells and 4 members in a span */
#define LX_BITS 32

#include <stdbool.h>
#include <stdint.h>
#include <setjmp.h>
#include <assert.h>

#include "../common.h"
#include "../lx.h"
#include "../memory.h"
#include "../ref.h"
#include "../list.h"

#define tag(type, cdr_code) \
	mktag(JOIN(cdr_,cdr_code), JOIN(lx_,JOIN(type,_tag)))

#define span(t0, d0, t1, d1, t2, d2, t3, d3) \
	tag_cell(t0, t1, t2, t3), d0, d1, d2, d3

#define int_tag(cdr) mktag(cdr, lx_int_tag)
#define lst_tag(cdr) mktag(cdr, lx_list_tag)
#define cdr_tag lst_tag(cdr_nil)
#define int_data(v) { .i = (lxint)v }
#define ref_data(cell_no, span_offset, offset) int_data( \
	(CELL_SIZE * (lxint)(SPAN_LENGTH*(span_offset) - (cell_no) - 1)) |\
	((lxint)(offset) & OFFSET_MASK) \
)

#define tag_cell(t0, t1, t2, t3) { { t0, t1, t2, t3 } }

#define mklist(cell, offset) \
	ref_to_list((struct lxref) { lx_list_tag, offset, cell })

static inline int assert_int_eq(lxint a, lxint b)
{
	if (a != b) fail_test("assertion failed: equal\n");
	return 0;
}

static inline int assert_eq(union lxvalue a, union lxvalue b)
{
	if (!lx_equals(a, b)) fail_test("assertion failed: equal\n");
	return 0;
}

static inline int assert_neq(union lxvalue a, union lxvalue b)
{
	if (lx_equals(a, b)) fail_test("assertion failed: not equal\n");
	return 0;
}

static inline int assert_list_eq(struct lxlist a, struct lxlist b)
{
	if (a.tag != b.tag || a.tag != lx_list_tag) {
		fail_test("assertion failed: tags are lists\n");
	}
	if (a.ref.cell != b.ref.cell || a.ref.offset != b.ref.offset) {
		fail_test("assertion failed: equal list identity\n");
	}
	return 0;
}

static inline int assert_list_neq(struct lxlist a, struct lxlist b)
{
	if (a.tag != b.tag || a.tag != lx_list_tag) {
		fail_test("assertion failed: tags are lists\n");
	}
	if (a.ref.cell == b.ref.cell && a.ref.offset == b.ref.offset) {
		fail_test("assertion failed: different list identity\n");
	}
	return 0;
}
