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
#define SHORT_LIST 40

struct wstate {
	struct lxref stack;
	struct lxmem *mem;
	char *p;
	lxint n, indent;
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

static bool put_ch(struct wstate *w, int ch)
{
	if (w->n == 0) { return false; }
	w->n--;
	*w->p++ = ch;
	return true;
}

static bool put_int(struct wstate *w, lxint val)
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
	if (i > w->n) { return false; }
	q = w->p;
	w->n -= i;
	w->p += i;
	while (i --> 0) {
		*q++ = buf[i];
	}
	return true;
}

static bool put_float(struct wstate *w, double val)
{
	int n;

	n = snprintf(w->p, w->n, "%g", val);
	if (n < 0) {
		abort();
	} else if (n > w->n) {
		return false;
	}
	w->p += n;
	w->n -= n;
	return true;
}

static bool put_raw_str(struct wstate *w, lxint n, char const *q)
{
	if (w->n < n) { return false; }
	(void)memcpy(w->p, q, n);
	w->n -= n;
	w->p += n;
	return true;
}

static bool put_str(struct wstate *w, lxint n, char const *q)
{
	lxint i, len;
	bool res;

	if (w->n < n) { return false; }

	if (n == 0) {
		/* empty string */
		return put_raw_str(w, 2, "\"\"");
	}
	len = (lxint)strcspn(q, "# (){}" ESCAPE_CHARS);
	if (len == n) {
		/* the whole string can be written unquoted */
		return put_raw_str(w, len, q);
	}

	/* the string must be written in quotes/special characters escaped */
	i = 0;
	res = put_ch(w, '"');
	while (res && i < n) {
		switch (q[i]) {
		case '"': i++; res = put_raw_str(w, 2, "\\\""); break;
		case '\\': i++; res = put_raw_str(w, 2, "\\\\"); break;
		case '\r': i++; res = put_raw_str(w, 2, "\\r"); break;
		case '\n': i++; res = put_raw_str(w, 2, "\\n"); break;
		case '\t': i++; res = put_raw_str(w, 2, "\\t"); break;
		case '\v': i++; res = put_raw_str(w, 2, "\\v"); break;
		default:
			len = (lxint)strcspn(q + i, ESCAPE_CHARS);
			res = put_raw_str(w, len, q + i);
			i += len;
			break;
		}
	}
	return res && put_ch(w, '"');
}

static bool write_brief(struct wstate *w, union lxvalue val, int maxdepth)
{
	bool res;
	struct lxlist lst;

	if (maxdepth == 0) {
		return false;
	}
	switch (val.tag) {
	default:
		return abort(), false;

	case lx_list_tag:
		if (lx_is_empty_list(val.list)) {
			return put_raw_str(w, 2, "()");
		} else {
			lst = val.list;
			val = lx_car(lst);
			res = put_ch(w, '(') &&
				write_brief(w, val, maxdepth - 1);
			lst = lx_cdr(lst);
			while (res && !lx_is_empty_list(lst)) {
				val = lx_car(lst);
				res = put_ch(w, ' ') &&
					write_brief(w, val, maxdepth - 1);
				lst = lx_cdr(lst);
			}
			return res && put_ch(w, ')');
		}

	case lx_tree_tag:
		if (lx_is_empty_tree(val.tree)) {
			return put_raw_str(w, 2, "{}");
		} else if (lx_tree_size(val.tree) == 1) {
			return put_ch(w, '{') &&
				write_brief(
					w,
					lx_list(lx_tree_entry(val.tree)),
					maxdepth - 1) &&
				put_ch(w, '}');
		}
		return false;

	case lx_bool_tag:
		return put_raw_str(w, 2, val.b ? "#t" : "#f");

	case lx_int_tag:
		return put_int(w, val.i);

	case lx_float_tag:
		return put_float(w, val.f);

	case lx_string_tag:
		return put_str(w, lx_strlen(val), val.s);
	}
}

static void write_value(struct wstate *w, union lxvalue val, bool break_lines)
{
	struct wstate u;

	if (w->indent > 0) {
		if (break_lines) {
			if (w->n < w->indent + 1) {
				out_of_memory(w);
			}
			*w->p = '\n';
			memset(w->p + 1, ' ', w->indent);
			w->n -= w->indent + 1;
			w->p += w->indent + 1;
		} else {
			if (!put_ch(w, ' ')) { out_of_memory(w); }
		}
	}
next:
	switch (val.tag) {
	default: abort();

	case lx_list_tag:
		if (lx_is_empty_list(val.list)) {
			if (!put_raw_str(w, 2, "()")) { out_of_memory(w); }
			break;
		} else {
tree_entry:
			if (break_lines) {
				u = *w;
				if (u.n > SHORT_LIST) {
					u.n = SHORT_LIST;
				}
				if (write_brief(&u, val, 10)) {
					w->n -= u.n;
					w->p = u.p;
					break;
				}
			}
			if (!put_ch(w, '(')) { out_of_memory(w); }
			w->indent++;
			push(w, lx_list(lx_cdr(val.list)));
			val = lx_car(val.list);
			goto next;
		}

	case lx_tree_tag:
		if (lx_is_empty_tree(val.tree)) {
			if (!put_raw_str(w, 2, "{}")) { out_of_memory(w); }
			break;
		} else {
			if (!put_ch(w, '{')) { out_of_memory(w); }
			w->indent++;
			push(w, lx_tree(lx_empty_tree()));
			push_tree_min(w, val.tree);
			val = pop(w);
			assert(val.tag == lx_tree_tag);
			push_tree_min(w, lx_tree_right(val.tree));
			val.list = lx_tree_entry(val.tree);
			assert(!lx_is_empty_list(val.list));
			goto tree_entry;
		}

	case lx_bool_tag:
		if (!put_raw_str(w, 2, val.b ? "#t" : "#f")) {
			out_of_memory(w);
		}
		break;

	case lx_int_tag:
		if (!put_int(w, val.i)) { out_of_memory(w); }
		break;

	case lx_float_tag:
		if (!put_float(w, val.f)) { out_of_memory(w); }
		break;

	case lx_string_tag:
		if (!put_str(w, lx_strlen(val), val.s)) { out_of_memory(w); }
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
				*next = lx_car(top.list);
				push(w, lx_list(lx_cdr(top.list)));
				return true;
			}
			if (!put_ch(w, ')')) { out_of_memory(w); }
			w->indent--;
		} else {
			assert(top.tag == lx_tree_tag);
			if (!lx_is_empty_tree(top.tree)) {
				/* we've visited the left sub-tree: visit this
				   entry next, and push everything up until the
				   minimum value of the right sub-tree */ 
				*next = lx_list(lx_tree_entry(top.tree));
				push_tree_min(w, lx_tree_right(top.tree));
				return true;
			}
			/* add closing bracket and pop more */
			if (!put_ch(w, '}')) { out_of_memory(w); }
			w->indent--;
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
	w.indent = 0;

	ref.tag = lx_string_tag;
	ref.offset = 0;
	ref.cell = mem->alloc.raw_free;

	val = value;
	do write_value(&w, val, false);
	while (next_value(&w, &val));

	put_ch(&w, '\0');

	/* store string length */
	mem->alloc.raw_free->i = w.p - begin - 1;

	/* advance raw free pointer */
	mem->alloc.raw_free += 1 + ceil_div(mem->alloc.raw_free->i, CELL_SIZE);

	return ref_to_string(ref);
}

union lxvalue lx_write_pretty(struct lxmem *mem, union lxvalue value)
{
	struct lxref ref;
	union lxvalue val;
	struct wstate w;
	char *begin;

	w.mem = mem;
	w.stack = mem->alloc.tag_free;
	w.p = begin = (char *)(mem->alloc.raw_free + 1);
	w.n = ((char *)mem->alloc.tag_free.cell - w.p) - 1;
	w.indent = 0;

	ref.tag = lx_string_tag;
	ref.offset = 0;
	ref.cell = mem->alloc.raw_free;

	val = value;
	do write_value(&w, val, true);
	while (next_value(&w, &val));

	put_ch(&w, '\0');

	/* store string length */
	mem->alloc.raw_free->i = w.p - begin - 1;

	/* advance raw free pointer */
	mem->alloc.raw_free += 1 + ceil_div(mem->alloc.raw_free->i, CELL_SIZE);

	return ref_to_string(ref);
}
