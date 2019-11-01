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
	struct lxvalue stack;
	struct lxmem *mem;
	char *p;
	lxint n, indent;
};

static void out_of_memory(struct wstate *w)
{
	longjmp(w->mem->escape, w->mem->oom); 
}

static void push(struct wstate *w, struct lxvalue val)
{
	union lxcell const *c;

	if (w->stack.offset) {
		w->stack.offset--;
	} else if (w->n > SPAN_LENGTH*CELL_SIZE) {
		c = ref_cell(w->stack) - SPAN_LENGTH;
		w->stack = mkref(lx_list_tag, OFFSET_MASK, c);
		w->n -= SPAN_LENGTH*CELL_SIZE;
	} else {
		out_of_memory(w);
	}
	*ref_tag(w->stack) = val.tag;
	lx_set_cell_data(ref_data(w->stack), val);
}

static struct lxvalue pop(struct wstate *w)
{
	struct lxvalue result;
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
		push(w, t.value);
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

static bool write_brief(struct wstate *w, struct lxvalue val, int maxdepth)
{
	bool res;
	struct lxlist list;
	struct lxtree tree;

	if (maxdepth == 0) {
		return false;
	}
	switch (val.tag) {
	default:
		return abort(), false;

	case lx_list_tag:
		list = ref_to_list(val);
		if (lx_is_empty_list(list)) {
			return put_raw_str(w, 2, "()");
		} else {
			val = lx_car(list);
			res = put_ch(w, '(') &&
				write_brief(w, val, maxdepth - 1);
			list = lx_cdr(list);
			while (res && !lx_is_empty_list(list)) {
				val = lx_car(list);
				res = put_ch(w, ' ') &&
					write_brief(w, val, maxdepth - 1);
				list = lx_cdr(list);
			}
			return res && put_ch(w, ')');
		}

	case lx_tree_tag:
		tree = ref_to_tree(val);
		if (lx_is_empty_tree(tree)) {
			return put_raw_str(w, 2, "{}");
		} else if (lx_tree_size(tree) == 1) {
			return put_ch(w, '{') &&
				write_brief(
					w,
					lx_tree_entry(tree).value,
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
		assert(val.offset == 0);
		return put_str(w, lx_strlen(ref_to_string(val)), val.s);
	}
}

static void write_value(struct wstate *w, struct lxvalue val, bool break_lines)
{
	struct wstate u;
	struct lxlist list;
	struct lxtree tree;

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
		list = ref_to_list(val);
		if (lx_is_empty_list(list)) {
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
			push(w, lx_cdr(list).value);
			val = lx_car(list);
			goto next;
		}

	case lx_tree_tag:
		tree = ref_to_tree(val);
		if (lx_is_empty_tree(tree)) {
			if (!put_raw_str(w, 2, "{}")) { out_of_memory(w); }
			break;
		} else {
			if (!put_ch(w, '{')) { out_of_memory(w); }
			w->indent++;
			push(w, lx_empty_tree().value);
			push_tree_min(w, tree);
			val = pop(w);
			tree = lx_tree(val);
			push_tree_min(w, lx_tree_right(tree));
			list = lx_tree_entry(tree);
			val = list.value;
			assert(!lx_is_empty_list(list));
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
		assert(val.offset == 0);
		if (!put_str(w, lx_strlen(ref_to_string(val)), val.s)) {
			out_of_memory(w);
		}
		break;
	}
}

/* fetch the next list or tree element, if any, from stack and write closing
   bracket(s) along the way */
static bool next_value(struct wstate *w, struct lxvalue *next)
{
	struct lxvalue top;
	struct lxlist list;
	struct lxtree tree;

	while (!is_empty_stack(w)) {
		top = pop(w);
		if (top.tag == lx_list_tag) {
			list = ref_to_list(top);
			if (!lx_is_empty_list(list)) {
				*next = lx_car(list);
				push(w, lx_cdr(list).value);
				return true;
			}
			if (!put_ch(w, ')')) { out_of_memory(w); }
			w->indent--;
		} else {
			assert(top.tag == lx_tree_tag);
			tree = ref_to_tree(top);
			if (!lx_is_empty_tree(tree)) {
				/* we've visited the left sub-tree: visit this
				   entry next, and push everything up until the
				   minimum value of the right sub-tree */ 
				*next = lx_tree_entry(tree).value;
				push_tree_min(w, lx_tree_right(tree));
				return true;
			}
			/* add closing bracket and pop more */
			if (!put_ch(w, '}')) { out_of_memory(w); }
			w->indent--;
		}
	}
	return false;
}

struct lxstring lx_write(struct lxmem *mem, struct lxvalue value)
{
	struct lxvalue ref;
	struct lxvalue val;
	struct wstate w;
	char *begin;

	w.mem = mem;
	w.stack = mem->alloc.tag_free;
	w.p = begin = (char *)(mem->alloc.raw_free + 1);
	w.n = ((char *)ref_cell(mem->alloc.tag_free) - begin) - 1;
	w.indent = 0;

	ref = mkref(lx_string_tag, 0, mem->alloc.raw_free + 1);

	val = value;
	do write_value(&w, val, false);
	while (next_value(&w, &val));

	put_ch(&w, '\0');

	/* store string length */
	mem->alloc.raw_free->i = w.p - begin - 1;

	/* advance raw free pointer */
	mem->alloc.raw_free += 1 + ceil_div(mem->alloc.raw_free->i + 1, CELL_SIZE);

	return ref_to_string(ref);
}

struct lxstring lx_write_pretty(struct lxmem *mem, struct lxvalue value)
{
	struct lxvalue ref;
	struct lxvalue val;
	struct wstate w;
	char *begin;

	w.mem = mem;
	w.stack = mem->alloc.tag_free;
	w.p = begin = (char *)(mem->alloc.raw_free + 1);
	w.n = ((char *)ref_cell(mem->alloc.tag_free) - begin) - 1;
	w.indent = 0;

	ref = mkref(lx_string_tag, 0, mem->alloc.raw_free + 1);

	val = value;
	do write_value(&w, val, true);
	while (next_value(&w, &val));

	put_ch(&w, '\0');

	/* store string length */
	mem->alloc.raw_free->i = w.p - begin - 1;

	/* advance raw free pointer */
	mem->alloc.raw_free += 1 + ceil_div(mem->alloc.raw_free->i + 1, CELL_SIZE);

	return ref_to_string(ref);
}
