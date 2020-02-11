#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>

#include "base/wbuf.h"
#include "../lx/common.h"
#include "../lx/lx.h"

#define cons(a, b) lx32_cons(m, a, b)

typedef struct lx32_mem mem;
typedef union lx32_value lxval;
typedef struct lx32_list list;
typedef struct lx32_tree tree;

static char const *argv0;

static lxval make_state(mem *m, tree env, list stack)
{
	return lx32_list(cons(lx32_tree(env), stack));
}

static lxval do_init(mem *m, lxval root, va_list ap)
{
	(void)ap;
	(void)root;
	return make_state(m, lx32_empty_tree(), lx32_empty_list());
}

static lxval do_read(mem *m, lxval state, va_list ap)
{
	struct lx32_read *result;
	char const *str;
	tree env;
	list stack;

	result = va_arg(ap, struct lx32_read *);
	str = va_arg(ap, char const *);

	env = lx32_car(state.list).tree;
	stack = lx32_cdr(state.list);

	*result = lx32_read(m, str);
	if (result->err == LX_READ_OK) {
		return make_state(m, env, cons(result->value, stack));
	} else {
		return state;
	}
}

static lxval do_write(mem *m, lxval state, va_list ap)
{
	list stack;
	tree env;
	lxval v;

	(void)ap;
	env = lx32_car(state.list).tree;
	stack = lx32_cdr(state.list);

	v = lx_write_pretty(m, lx32_car(stack));

	return make_state(m, env, cons(v, lx32_cdr(stack)));
}

static void handle_result(struct lx32_result result)
{
	if (result.status) {
		fprintf(stderr, "%s: out of memory\n", argv0);
	}
}

static bool handle_read(struct lx32_read const *result, struct wbuf *buf)
{
	size_t len;

	switch (result->err) {
	case LX_READ_OK:
 		return true;
	case LX_READ_INCOMPLETE:
		len = result->where - (char const *)buf->end;
		wbuf_alloc(buf, len);
		return false;
		
	case LX_READ_UNEXPECTED:
		fprintf(stderr, "Unexpected: %c\n", result->where[0]);
		break;
		
	case LX_READ_SHARP:
		fprintf(stderr, "Unexpected boolean: %c\n", *result->where);
		break;
	case LX_READ_STRING:
		fprintf(stderr, "Unexpected boolean: %c\n", *result->where);
		break;
	case LX_READ_NUMBER:
		fprintf(stderr, "Unexpected numeric literal: %c\n", *result->where);
		break;
	case LX_READ_ENTRY:
		fprintf(stderr, "Tree entries must be lists: %s\n", result->where);
		break;
	default:
		abort();
	}
	wbuf_rewind(buf);
	return false;
}

static void maybe_grow_buffer(struct wbuf *buf)
{
	if (wbuf_available(buf) < 80) {
		if (wbuf_reserve(buf, 80)) {
			fprintf(stderr, "%s: out of memory\n", argv0);
		}
	}
}

int main(int argc, char **argv)
{
	struct lx32_result result;
	struct lx32_read read;
	struct lx32_heap *heap;
	struct wbuf input;
	FILE *fp;
	lxval v;

	heap = lx32_make_heap(0, 0);
	fp = stdin;
	argv0 = argc > 0 ? argv[0] : "devise";

	wbuf_init(&input);
	wbuf_reserve(&input, 80);
	result = lx32_modifyl(heap, do_init);
	handle_result(result);

	while (fgets(input.end, wbuf_available(&input), fp)) {
		result = lx32_modifyl(heap, do_read, &read, (char *)input.end);
		handle_result(result);
		if (!handle_read(&read, &input)) {
			maybe_grow_buffer(&input);
			continue;
		}
		result = lx32_modifyl(heap, do_write);
		handle_result(result);

		v = lx32_car(lx32_cdr(result.value.list));
		assert (v.tag == lx_string_tag);
		puts(v.s);

		wbuf_consume(&input, read.where - (char const *)input.begin);
	}
}
