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

enum state {
	top_level,
	list_member,
	map_key,
	map_value,
};

struct wstate {
	struct lxvalue stack;
	struct lxstate *s;
	char *begin, *p;
	lxint n, offset, indent;
	enum state state;
};

static lxint char_offset(struct wstate *w) {
	return w->p - w->begin;
}

static void out_of_memory(struct wstate *w)
{
	lx_handle_out_of_memory(w->s);
}

static void push(struct wstate *w, struct lxvalue val, unsigned loc)
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
	*ref_tag(w->stack) = mktag(loc, val.tag);
	lx_set_cell_data(ref_data(w->stack), val);
}

static struct lxvalue pop(struct wstate *w, unsigned *loc)
{
	struct lxvalue result;
	result = lx_car(ref_to_list(w->stack));
	*loc = lxtag_len(*ref_tag(w->stack));
	w->stack = forward(w->stack);
	if (w->stack.offset == 0) {
		w->n += SPAN_LENGTH*CELL_SIZE;
	}
	return result;
}

static bool is_empty_stack(struct wstate *w)
{
	return ref_eq(w->s->alloc.tag_free, w->stack);
}

static void push_map_min(struct wstate *w, struct lxmap map)
{
	struct lxmap p;

	p = map;
	while (!lx_is_empty_map(p)) {
		push(w, lx_valuei(w->indent), w->state);
		push(w, p.value, 0);
		p = lx_map_left(p);
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

	// keywords must be quoted
	if (strcmp(q, "nil") == 0) {
		return put_raw_str(w, 5, "\"nil\"");
	}

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
	struct lxvalue key, value;
	struct lxlist list;
	struct lxmap map;

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

	case lx_map_tag:
		map = ref_to_map(val);
		if (lx_is_empty_map(map)) {
			return put_raw_str(w, 2, "{}");
		} else if (lx_map_size(map) == 1) {
			list = lx_map_entry(map);
			key = lx_car(list);
			value = lx_car(lx_cdr(list));
			return put_ch(w, '{') &&
				write_brief(w, key, maxdepth - 1) &&
				put_ch(w, ' ') &&
				write_brief(w, value, maxdepth - 1) &&
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

	case lx_nil_tag:
		return put_raw_str(w, 3, "nil");
	}
}

static void newline_indent(struct wstate *w, lxint indent)
{
	if (w->n < indent + 1) {
		out_of_memory(w);
	}
	*w->p = '\n';
	memset(w->p + 1, ' ', indent);
	w->n -= indent + 1;
	w->p += indent + 1;
}

static void write_space(struct wstate *w, bool break_lines)
{
	unsigned offset, diff;

	if (!break_lines) {
		if (w->state == map_key) {
			if (!put_ch(w, ' ')) { out_of_memory(w); }
		}
		if (!put_ch(w, ' ')) { out_of_memory(w); }
		return;
	}

	if (w->state == map_value) {
		offset = char_offset(w);
		diff = offset - w->offset;
		if (diff > 7) {
			w->indent += 2;
			newline_indent(w, w->indent);
		} else {
			w->indent += diff + 1;
			if (!put_ch(w, ' ')) { out_of_memory(w); }
		}
	} else {
		newline_indent(w, w->indent);
	}
}

static void write_value(struct wstate *w, struct lxvalue val, bool break_lines)
{
	struct wstate u;
	struct lxlist list;
	struct lxmap map;
	unsigned ignore;

	if (w->indent > 0) {
		write_space(w, break_lines);
	}

next:
	if (w->state == map_key) {
		w->offset = char_offset(w);
	}
	switch (val.tag) {
	default: abort();

	case lx_list_tag:
		list = ref_to_list(val);
		if (lx_is_empty_list(list)) {
			if (!put_raw_str(w, 2, "()")) { out_of_memory(w); }
			break;
		} else {
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
			push(w, lx_valuei(char_offset(w)), 0);
			push(w, lx_valuei(w->indent), w->state);
			push(w, lx_cdr(list).value, 0);
			w->indent++;
			w->state = list_member;
			w->offset = char_offset(w);
			val = lx_car(list);
			goto next;
		}

	case lx_map_tag:
		map = ref_to_map(val);
		if (lx_is_empty_map(map)) {
			if (!put_raw_str(w, 2, "{}")) { out_of_memory(w); }
			break;
		} else {
			if (!put_ch(w, '{')) { out_of_memory(w); }

			/* Save key offset and write state, popped by pop_value
			   below */
			push(w, lx_valuei(char_offset(w)), 0);
			push(w, lx_valuei(w->indent), w->state);

			w->indent++;
			w->state = map_key;

			/* push empty map as marker for end-of-map */
			push(w, lx_empty_map().value, 0);

			/* get min value */
			push_map_min(w, map);

			val = pop(w, &ignore);
			map = lx_map(val);
			assert(lx_is_empty_map(lx_map_left(map)));
			pop(w, &ignore); /* remove indentation */

			/* push path to second to smallest value */
			push_map_min(w, lx_map_right(map));

			list = lx_map_entry(map);
			val = lx_car(list);
			push(w, map.value, 1);
			goto next;
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

	case lx_nil_tag:
		if (!put_raw_str(w, 3, "nil")) {
			out_of_memory(w);
		}
		break;
	}
}

/* fetch the next list or map element, if any, from stack and write closing
   bracket(s) along the way */
static bool pop_value(struct wstate *w, struct lxvalue *next)
{
	struct lxvalue top;
	struct lxlist list;
	struct lxmap map;
	unsigned loc;

	while (!is_empty_stack(w)) {
		top = pop(w, &loc);
		if (top.tag == lx_list_tag) {
			assert(loc == 0);
			/* restore state after last value was printed */
			assert(w->state == list_member);
			list = ref_to_list(top);
			if (!lx_is_empty_list(list)) {
				*next = lx_car(list);
				push(w, lx_cdr(list).value, 0);
				return true;
			}
			if (!put_ch(w, ')')) { out_of_memory(w); }
		} else {
			assert(top.tag == lx_map_tag);
			map = ref_to_map(top);
			if (!lx_is_empty_map(map)) {
				if (loc == 0) {
					/* restore indentation which might have
					   been altered for the value */
					top = pop(w, &loc);
					w->indent = top.i;

					/* put tree node back on the stack so
					   that it can be visited again to
					   write the value */
					w->state = map_key;
					*next = lx_car(lx_map_entry(map));
					push(w, map.value, 1);
				} else {
					assert(loc == 1);
					/* second visit to this node: write the
					   value */
					w->state = map_value;
					*next = lx_nth(lx_map_entry(map), 1);

					/* we've visited the left sub-tree and
					   written the key, the value of this
					   entry will be written next, and then
					   let's visit right sub-tree */
					push_map_min(w, lx_map_right(map));
				}
				return true;
			}
			/* add closing bracket and pop more */
			if (!put_ch(w, '}')) { out_of_memory(w); }
		}
		top = pop(w, &loc);
		w->indent = top.i;
		w->state = loc;
		top = pop(w, &loc);
		w->offset = top.i;
	}
	return false;
}

static struct lxstring do_write(
	struct lxstate *s,
	struct lxvalue value,
	bool break_lines)
{
	struct lxvalue ref;
	struct lxvalue val;
	struct wstate w;
	char *begin;

	w.s = s;
	w.stack = s->alloc.tag_free;
	w.stack.tag = lx_list_tag;
	w.begin = w.p = begin = (char *)(s->alloc.raw_free + 1);
	w.n = ((char *)ref_cell(s->alloc.tag_free) - begin) - 1;
	w.indent = 0;
	w.state = top_level;

	ref = mkref(lx_string_tag, 0, s->alloc.raw_free + 1);

	val = value;
	do write_value(&w, val, break_lines);
	while (pop_value(&w, &val));

	put_ch(&w, '\0');

	/* store string length */
	s->alloc.raw_free->i = w.p - begin - 1;

	/* advance raw free pointer */
	s->alloc.raw_free += 1 + ceil_div(s->alloc.raw_free->i + 1, CELL_SIZE);

	return ref_to_string(ref);
}

struct lxstring lx_write(struct lxstate *s, struct lxvalue value)
{
	return do_write(s, value, false);
}

struct lxstring lx_write_pretty(struct lxstate *s, struct lxvalue value)
{
	return do_write(s, value, true);
}
