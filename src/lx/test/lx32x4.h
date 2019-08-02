/* 32-bit cells and 4 members in a span */
#define LX_BITS 32

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <setjmp.h>
#include <assert.h>

#include "../common.h"
#include "../lx.h"
#include "../memory.h"
#include "../ref.h"
#include "../list.h"

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

#define length_of(x) (sizeof (x) / sizeof *(x))

#define tag(type, len) mktag(len, JOIN(lx_,JOIN(type,_tag)))

#define span(t0, d0, t1, d1, t2, d2, t3, d3) \
	tag_cell(t0, t1, t2, t3), d0, d1, d2, d3

#define int_tag(len) mktag(len, lx_int_tag)
#define lst_tag(len) mktag(len, lx_list_tag)
#define cdr_tag lst_tag(1)
#define nil_data int_data(0)
#define int_data(v) { .i = (lxint)v }
#define ref_data(cell_no, span_offset, offset) int_data( \
	(CELL_SIZE * (lxint)(SPAN_LENGTH*(span_offset) - (cell_no) - 1)) |\
	((lxint)(offset) & OFFSET_MASK) \
)

#define tag_cell(t0, t1, t2, t3) { { t0, t1, t2, t3 } }

#define mklist(cell, offset) \
	ref_to_list((struct lxref) { lx_list_tag, offset, cell })

struct assert_ctx
{
	char const *value, *expected, *size, *file, *func;
	unsigned line;
};

#define assert_ctx(val, exp) \

#define _call_assert(name, val, exp) \
	name(val, exp, (struct assert_ctx){ \
		#val, #exp, "", __FILE__, __func__, __LINE__ \
	})

#define _call_assert_sz(name, val, exp, size) \
	name(val, exp, size, (struct assert_ctx){ \
		#val, #exp, #size, __FILE__, __func__, __LINE__ \
	})

#define assertion_failed(ctx) \
	printf("%s:%s:%u: Assertion %s == %s failed!\n", \
		ctx.file, ctx.func, ctx.line, ctx.value, ctx.expected)

static inline int _assert_int_eq(
	lxint value,
	lxint expected,
	struct assert_ctx ctx)
{
	if (value == expected) { return 0; }
	assertion_failed(ctx);
	printf("Got: %d\nExpected: %d\n", value, expected);
	fail_test(0);
	return -1;
}

#define assert_int_eq(value, expected) \
	_call_assert(_assert_int_eq, value, expected)

static inline void _assert_tag_eq(
	enum lx_tag value,
	enum lx_tag expected,
	struct assert_ctx ctx)
{
	if (value == expected) { return; }
	assertion_failed(ctx);
	printf("Got: %d\nExpected: %d\n", value, expected);
	fail_test(0);
}

#define assert_tag_eq(value, expected) \
	_call_assert(_assert_tag_eq, value, expected)

static inline void _assert_status_eq(
	int value,
	int expected,
	struct assert_ctx ctx)
{
	if (value == expected) { return; }
	assertion_failed(ctx);
	printf("Got: %d\nExpected: %d\n", value, expected);
	fail_test(0);
}

#define assert_status_eq(value, expected) \
	_call_assert(_assert_status_eq, value, expected)

static inline int _assert_ptr_eq(
	union lxcell const *a,
	union lxcell const *b,
	struct assert_ctx ctx)
{
	if (a == b) { return 0; }
	assertion_failed(ctx);
	printf("Got: %p\nExpected: %p\n", (void *)a, (void *)b);
	fail_test(0);
	return -1;
}

#define assert_ptr_eq(value, expected) \
	_call_assert(_assert_ptr_eq, value, expected)

static inline int _assert_ref_eq(
	struct lxref a,
	struct lxref b,
	struct assert_ctx ctx)
{
	if (a.cell == b.cell && a.offset == b.offset) {
		return 0;
	}
	assertion_failed(ctx);
	printf("Got: (%p, %d)\nExpected: (%p, %d)\n",
		(void *)a.cell, a.offset,
		(void *)b.cell, b.offset);
	fail_test(0);
	return -1;
}

#define assert_ref_eq(value, expected) \
	_call_assert(_assert_ref_eq, value, expected)

static void _print_value(union lxvalue val)
{
	struct lxlist l;
	switch (val.tag) {
	default: abort();
	case lx_list_tag:
		printf("(");
		while (!lx_is_empty_list(l)) {
			_print_value(lx_car(val.list));
			l = lx_cdr(val.list);
			if (!lx_is_empty_list(l)) { printf(" "); }
		}
		printf(")");
		break;
	case lx_bool_tag:
		if (val.i) {
			printf("#t");
		} else {
			printf("#f");
		}
		break;
	case lx_int_tag:
		printf("%d", val.i);
		break;
	case lx_float_tag:
		printf("%f", val.f);
		break;
	}
}

static inline int _assert_eq(
	union lxvalue a,
	union lxvalue b,
	struct assert_ctx ctx)
{
	if (lx_equals(a, b)) {
		return 0;
	}
	printf("%s:%s:%u: Assertion " QUOTE(lx_equals) "(%s, %s) failed!\n", \
		ctx.file, ctx.func, ctx.line, ctx.value, ctx.expected);
	printf("Got: ");
	_print_value(a);
	printf("\nExpected: ");
	_print_value(b);
	printf("\n");
	fail_test(0);
	return -1;
}

#define assert_eq(value, expected) \
	_call_assert(_assert_eq, value, expected)

static inline int _assert_neq(
	union lxvalue a,
	union lxvalue b,
	struct assert_ctx ctx)
{
	if (!lx_equals(a, b)) {
		return 0;
	}
	printf("%s:%s:%u: Assertion !" QUOTE(lx_equals) "(%s, %s) failed!\n", \
		ctx.file, ctx.func, ctx.line, ctx.value, ctx.expected);
	printf("Got: ");
	_print_value(a);
	printf("\n");
	fail_test(0);
	return -1;

}

#define assert_neq(value, expected) \
	_call_assert(_assert_neq, value, expected)

static inline int _assert_list_eq(
	struct lxlist a,
	struct lxlist b,
	struct assert_ctx ctx)
{
	if (a.tag != lx_list_tag) {
		assertion_failed(ctx);
		printf("%s is not a list (got %d)\n", ctx.value, (int)a.tag);
		fail_test(0);
	}
	if (b.tag != lx_list_tag) {
		assertion_failed(ctx);
		printf("%s is not a list (got %d)\n", ctx.expected, (int)b.tag);
		fail_test(0);
	}
	if (!list_eq(a, b)) {
		assertion_failed(ctx);
		printf("Got: (%p, %d)\nExpected: (%p, %d)\n",
			(void *)a.ref.cell, a.ref.offset,
			(void *)b.ref.cell, b.ref.offset);
		fail_test(0);
	}
	return 0;
}

#define assert_list_eq(value, expected) \
	_call_assert(_assert_list_eq, value, expected)

static inline int _assert_list_neq(
	struct lxlist a,
	struct lxlist b,
	struct assert_ctx ctx)
{
	if (a.tag != lx_list_tag) {
		printf("%s:%s:%u: Assertion %s != %s  failed!\n",
			ctx.file, ctx.func, ctx.line, ctx.value, ctx.expected);
		printf("%s is not a list (got %d)\n", ctx.value, (int)a.tag);
		fail_test(0);
	}
	if (b.tag != lx_list_tag) {
		printf("%s:%s:%u: Assertion %s != %s  failed!\n",
			ctx.file, ctx.func, ctx.line, ctx.value, ctx.expected);
		printf("%s is not a list (got %d)\n", ctx.expected, (int)b.tag);
		fail_test(0);
	}
	if (a.ref.cell == b.ref.cell && a.ref.offset == b.ref.offset) {
		printf("%s:%s:%u: Assertion %s != %s  failed!\n",
			ctx.file, ctx.func, ctx.line, ctx.value, ctx.expected);
		fail_test(0);
	}
	return 0;
}

#define assert_list_neq(value, expected) \
	_call_assert(_assert_list_neq, value, expected)

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
	case lx_list_tag:
		_append_char('(', p, size);
		if (lx_is_empty_list(value.list)) {
			_append_char(')', p, size);
			break;
		}
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

static inline char *serialize(union lxvalue value, char *p, size_t size)
{
	size_t sz = size;
	char *q = p;
	if (size == 0) { return NULL; }
	serialize_rec(value, &q, &sz);
	*q = '\0';
	return p;
}

static inline void _assert_serialize_eq(
	union lxvalue a,
	char const *str,
	struct assert_ctx ctx)
{
	char buf[500];

	serialize(a, buf, sizeof buf);
	if (strcmp(buf, str) == 0) { return; }
	printf("%s:%s:%u: Assertion serialize(%s) == %s failed!\n", \
		ctx.file, ctx.func, ctx.line, ctx.value, str);
	printf("Got: %s\n", buf);
	fail_test(0);
}

#define assert_serialize_eq(value, expected) \
	_call_assert(_assert_serialize_eq, value, expected)

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
	struct assert_ctx ac)
{
	if (memcmp(value, expected, size) == 0) { return; }
	printf("%s:%s:%u: Assertion memcmp(%s, %s, %s) == 0 failed!\n",
		ac.file, ac.func, ac.line, ac.value, ac.expected, ac.size);
	printf("Got:\n");
	_print_hex(stdout, value, size);
	printf("Expected:\n");
	_print_hex(stdout, expected, size);
	fail_test(0);
}

#define assert_mem_eq(value, expected, size) \
	_call_assert_sz(_assert_mem_eq, value, expected, size)

static inline void _assert_str_eq(
	char const *value,
	char const *expected,
	struct assert_ctx ctx)
{
	if (value && expected && strcmp(value, expected) == 0) { return; }
	printf("%s:%s:%u: Assertion strcmp(%s, %s) == 0 failed!\n",
		ctx.file, ctx.func, ctx.line, ctx.value, ctx.expected);
	printf("Got:\n%s\nExpected:\n%s\n", value, expected);
	fail_test(0);
}

#define assert_str_eq(value, expected) \
	_call_assert(_assert_str_eq, value, expected)
