/* 32-bit cells and 4 members in a span */
#define LX_BITS 32
#define CELL_SPAN 4

#include "../lx.c"

#define JOIN(a, b) JOIN_(a, b)

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
	(struct lx_list) { lx_list_tag, offset, cell }

#define empty_list mklist(NULL, 0)

static inline int assert_int_eq(lxint a, lxint b)
{
	if (a == b) return 0; else return ok = -1;
}

static inline int assert_eq(union lxvalue a, union lxvalue b)
{
	if (lx_equals(a, b)) return 0; else return ok = -1;
}

static inline int assert_neq(union lxvalue a, union lxvalue b)
{
	if (lx_equals(a, b)) return ok = -1; else return 0;
}

static inline int assert_list_eq(struct lx_list a, struct lx_list b)
{
	if (list_eq(a, b)) return 0; else return ok = -1;
}

static inline int assert_list_neq(struct lx_list a, struct lx_list b)
{
	if (list_eq(a, b)) return ok = -1; else return 0;
}
