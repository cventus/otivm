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

static inline int _assert_int_eq(
	lxint value,
	lxint expected,
	char const *value_exp,
	char const *expected_exp,
	char const *file,
	unsigned int line,
	char const *func)
{
	if (value == expected) { return 0; }
	printf("%s:%s:%u: Assertion %s == %s failed!\n",
		file, func, line, value_exp, expected_exp);
	printf("Got: %d\nExpected: %d\n", value, expected);
	fail_test(0);
	return -1;
}

#define assert_int_eq(value, expected) \
	_assert_int_eq(value, expected, \
	 #value,  #expected, \
	 __FILE__, __LINE__, __func__)

static inline void _assert_tag_eq(
	enum lx_tag value,
	enum lx_tag expected,
	char const *value_exp,
	char const *expected_exp,
	char const *file,
	unsigned int line,
	char const *func)
{
	if (value == expected) { return; }
	printf("%s:%s:%u: Assertion %s == %s failed!\n",
		file, func, line, value_exp, expected_exp);
	printf("Got: %d\nExpected: %d\n", value, expected);
	fail_test(0);
}

#define assert_tag_eq(value, expected) \
	_assert_tag_eq(value, expected, \
	 #value,  #expected, \
	 __FILE__, __LINE__, __func__)

static inline int assert_ptr_eq(union lxcell const *a, union lxcell const *b)
{
	if (a != b) { fail_test("assertion failed: equal\n"); }
	return 0;
}

static inline int assert_ref_eq(struct lxref a, struct lxref b)
{
	if (a.cell != b.cell || a.offset != b.offset) {
		fail_test("assertion failed: equal\n");
	}
	return 0;
}

static inline int assert_eq(union lxvalue a, union lxvalue b)
{
	if (!lx_equals(a, b)) { fail_test("assertion failed: equal\n"); }
	return 0;
}

static inline int assert_neq(union lxvalue a, union lxvalue b)
{
	if (lx_equals(a, b)) { fail_test("assertion failed: not equal\n"); }
	return 0;
}

static inline int assert_list_eq(struct lxlist a, struct lxlist b)
{
	if (a.tag != b.tag || !(a.tag == lx_list_tag || a.tag == lx_nil_tag)) {
		fail_test("assertion failed: tags are lists\n");
	}
	if (!list_eq(a, b)) {
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

static inline void _append_char(int ch, char **p, size_t *size)
{
	if (*size > 0) {
		--*size;
		if (*size > 0) {
			*(*p)++ = ch;
		}
	}
}

static inline void serialize_rec(union lxvalue value, char **p, size_t *size)
{
	struct lxlist l;
	int n;

	if (*size == 0) { return; }

	switch (value.tag) {
	case lx_nil_tag:
		_append_char('(', p, size);
		_append_char(')', p, size);
		break;
	case lx_list_tag:
		_append_char('(', p, size);
		serialize_rec(lx_car(value.list), p, size);
		l = lx_cdr(value.list);
		while (!lx_is_empty_list(l)) {
			_append_char(' ', p, size);
			serialize_rec(lx_car(l), p, size);
			l = lx_cdr(l);
		}
		_append_char(')', p, size);
		break;
	case lx_int_tag:
		n = snprintf(*p, *size, "%d", value.i);
		if (n < 0) { n = 0; }
		if ((size_t)n > *size) { n = *size; }
		*size -= n;
		*p += n;
		break;
	default:
		abort();
		break;
	}
}

static inline void serialize(union lxvalue value, char *p, size_t size)
{
	size_t sz = size;
	char *q = p;
	if (size == 0) { return; }
	serialize_rec(value, &q, &sz);
	*q = '\0';
}

static inline void _print_hex(FILE *fp, void const *data, size_t size)
{
	size_t i;
	unsigned char const *p  = data;
	for (i = 0; i < size; i++) {
		if (i > 0) {
			if (i % 8 == 0) {
				fputc('\n', fp);
			} else if (i % 2 == 0) {
				fputc(' ', fp);
				if (i % 4 == 0) {
					fputc(' ', fp);
				}
			}
		}
		fprintf(fp, "%02x", *p++);
	}
	fputc('\n', fp);
}

static inline void _assert_mem_eq(
	void const *value,
	void const *expected,
	size_t size,
	char const *value_exp,
	char const *expected_exp,
	char const *size_exp,
	char const *file,
	unsigned int line,
	char const *func)
{
	if (memcmp(value, expected, size) == 0) { return; }
	printf("%s:%s:%u: Assertion memcmp(%s, %s, %s) != 0 failed!\n",
		file, func, line, value_exp, expected_exp, size_exp);
	printf("Got:\n");
	_print_hex(stdout, value, size);
	printf("Expected:\n");
	_print_hex(stdout, expected, size);
	fail_test(0);
}

#define assert_mem_eq(value, expected, size) \
	_assert_mem_eq(value, expected, size, \
	 #value,  #expected, #size, \
	 __FILE__, __LINE__, __func__)

static inline void _assert_str_eq(
	char const *value,
	char const *expected,
	char const *value_exp,
	char const *expected_exp,
	char const *file,
	unsigned int line,
	char const *func)
{
	if (strcmp(value, expected) == 0) { return; }
	printf("%s:%s:%u: Assertion strcmp(%s, %s) != 0 failed!\n",
		file, func, line, value_exp, expected_exp);
	printf("Got:\n%s\nExpected:\n%s\n", value, expected);
	fail_test(0);
}

#define assert_str_eq(value, expected) \
	_assert_str_eq(value, expected, \
	 #value,  #expected, \
	 __FILE__, __LINE__, __func__)
