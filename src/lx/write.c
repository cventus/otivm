#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>
#include <setjmp.h>
#include <inttypes.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "str.h"
#include "list.h"
#include "tree.h"

#define ESCAPE_CHARS "\"\\\r\n\t\v"

struct wstate {
	struct lxref stack;
	struct lxmem *mem;
	char *p;
	lxint n;
};

static void out_of_memory(struct wstate *w)
{
	longjmp(w->mem->escape, w->mem->oom); 
}

static void push(struct wstate *w, union lxvalue val)
{
	if (w->stack.offset) {
		w->stack.offset--;
	} else if (w->n > SPAN_LENGTH*CELL_SIZE) {
		w->stack.offset = OFFSET_MASK;
		w->stack.cell -= SPAN_LENGTH;
		w->n -= SPAN_LENGTH*CELL_SIZE;
	} else {
		out_of_memory(w);
	}
	lx_set_cell_data(ref_data(w->stack), val);
	*ref_tag(w->stack) = val.tag;
}

static union lxvalue pop(struct wstate *w)
{
	union lxvalue result;
	result = lx_car(ref_to_list(w->stack));
	w->stack = forward(w->stack);
	if (w->stack.offset == 0) {
		w->n += SPAN_LENGTH*CELL_SIZE;
	}
	return result;
}

static bool is_empty_stack(struct wstate *w)
{
	return ref_eq(w->mem->alloc.tag_free, w->stack);
}

static void push_tree_min(struct wstate *w, struct lxtree tree)
{
	struct lxtree t;

	t = tree;
	while (!lx_is_empty_tree(t)) {
		push(w, lx_tree(t));
		t = lx_tree_left(t);
	}
}

static void put_ch(struct wstate *w, int ch)
{
	if (w->n == 0) { out_of_memory(w); }
	w->n--;
	*w->p++ = ch;
}

static void put_int(struct wstate *w, lxint val)
{
	char buf[LX_BITS + 1], *q;
	int i;
	lxint a;

	if (val == 0) {
		buf[0] = '0';
		i = 1;
	} else {
		i = 0;
		a = val;
		if (a < 0) {
			if (a == JOIN(INT, JOIN(LX_BITS, _MIN))) {
				/* -2^(2^n - 1) ends with an 8 for n > 1 */
				a++;
				a = -a;
				buf[i++] = '0' + (a % 10) + 1;
				a /= 10;
			} else {
				a = -a;
			}
		}
		while (a > 0) {
			buf[i++] = '0' + (a % 10);
			a /= 10;
		}
		if (val < 0) {
			buf[i++] = '-';
		}
	}
	if (i > w->n) { out_of_memory(w); }
	q = w->p;
	w->n -= i;
	w->p += i;
	while (i --> 0) {
		*q++ = buf[i];
	}
}

static void put_float(struct wstate *w, double val)
{
	int n;

	n = snprintf(w->p, w->n, "%g", val);
	if (n < 0) {
		abort();
	} else if (n > w->n) {
 		out_of_memory(w); 
	}
	w->p += n;
	w->n -= n;
}

static void put_raw_str(struct wstate *w, lxint n, char const *q)
{
	if (w->n < n) { out_of_memory(w); }
	(void)memcpy(w->p, q, n);
	w->n -= n;
	w->p += n;
}

static void put_str(struct wstate *w, lxint n, char const *q)
{
	lxint i, len;

	if (w->n < n) { out_of_memory(w); }

	if (n == 0) {
		/* empty string */
		put_raw_str(w, 2, "\"\"");
		return;
	}
	len = (lxint)strcspn(q, "# (){}" ESCAPE_CHARS);
	if (len == n) {
		/* the whole string can be written unquoted */
		put_raw_str(w, len, q);
		return;
	}

	/* the string must be written in quotes/special characters escaped */
	i = 0;
	put_ch(w, '"');
	while (i < n) {
		switch (q[i]) {
		case '"': i++; put_raw_str(w, 2, "\\\""); break;
		case '\\': i++; put_raw_str(w, 2, "\\\\"); break;
		case '\r': i++; put_raw_str(w, 2, "\\r"); break;
		case '\n': i++; put_raw_str(w, 2, "\\n"); break;
		case '\t': i++; put_raw_str(w, 2, "\\t"); break;
		case '\v': i++; put_raw_str(w, 2, "\\v"); break;
		default:
			len = (lxint)strcspn(q + i, ESCAPE_CHARS);
			put_raw_str(w, len, q + i);
			i += len;
			break;
		}
	}
	put_ch(w, '"');
}

static void write_value(struct wstate *w, union lxvalue val)
{
next:
	switch (val.tag) {
	default: abort();

	case lx_list_tag:
		if (lx_is_empty_list(val.list)) {
			put_raw_str(w, 2, "()");
			break;
		} else {
tree_entry:
			put_ch(w, '(');
			push(w, lx_list(lx_cdr(val.list)));
			val = lx_car(val.list);
			goto next;
		}

	case lx_tree_tag:
		if (lx_is_empty_tree(val.tree)) {
			put_raw_str(w, 2, "{}");
			break;
		} else {
			put_ch(w, '{');
			push(w, lx_tree(lx_empty_tree()));
			push_tree_min(w, val.tree);
			val = pop(w);
			assert(val.tag == lx_tree_tag);
			push_tree_min(w, lx_tree_right(val.tree));
			val.list = lx_tree_entry(val.tree);
			goto tree_entry;
		}

	case lx_bool_tag:
		put_raw_str(w, 2, val.b ? "#t" : "#f");
		break;

	case lx_int_tag:
		put_int(w, val.i);
		break;

	case lx_float_tag:
		put_float(w, val.f);
		break;

	case lx_string_tag:
		put_str(w, lx_strlen(val), val.s);
		break;
	}
}

/* fetch the next list or tree element, if any, from stack and write closing
   bracket(s) along the way */
static bool next_value(struct wstate *w, union lxvalue *next)
{
	union lxvalue top;

	while (!is_empty_stack(w)) {
		top = pop(w);
		if (top.tag == lx_list_tag) {
			if (!lx_is_empty_list(top.list)) {
				put_ch(w, ' ');
				*next = lx_car(top.list);
				push(w, lx_list(lx_cdr(top.list)));
				return true;
			}
			put_ch(w, ')');
		} else {
			assert(top.tag == lx_tree_tag);
			if (!lx_is_empty_tree(top.tree)) {
				/* we've visited the left sub-tree: visit this
				   entry next, and push everything up until the
				   minimum value of the right sub-tree */ 
				put_ch(w, ' ');
				*next = lx_list(lx_tree_entry(top.tree));
				push_tree_min(w, lx_tree_right(top.tree));
				return true;
			}
			/* add closing bracket and pop more */
			put_ch(w, '}');
		}
	}
	return false;
}

union lxvalue lx_write(struct lxmem *mem, union lxvalue value)
{
	struct lxref ref;
	union lxvalue val;
	struct wstate w;
	char *begin;

	w.mem = mem;
	w.stack = mem->alloc.tag_free;
	w.p = begin = (char *)(mem->alloc.raw_free + 1);
	w.n = ((char *)mem->alloc.tag_free.cell - w.p) - 1;
	ref.tag = lx_string_tag;
	ref.offset = 0;
	ref.cell = mem->alloc.raw_free;

	val = value;
	do write_value(&w, val);
	while (next_value(&w, &val));

	put_ch(&w, '\0');

	/* store string length */
	mem->alloc.raw_free->i = w.p - begin - 1;

	/* advance raw free pointer */
	mem->alloc.raw_free += 1 + ceil_div(mem->alloc.raw_free->i, CELL_SIZE);

	return ref_to_string(ref);
}
