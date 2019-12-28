#include <stdarg.h>
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
#include "alloc.h"
#include "str.h"
#include "list.h"
#include "tree.h"

static struct lxread read_ok(struct lxvalue result, char const *where) {
	return (struct lxread) { LX_READ_OK, where, result };
}

static struct lxread read_err(enum lx_read_status status, char const *where) {
	return (struct lxread) { status, where, lx_valueb(false) };
}

#define WHITESPACE " \t\v\r\n"
#define SEPARATORS WHITESPACE "(){}#\""

static char const *skip_ws(char const *str)
{
	char const *p = str;
	while (*p && strchr(WHITESPACE, *p)) { p++; }
	return p;
}

static bool is_separator(int ch)
{
	return ch == 0 || strchr(SEPARATORS, ch);
}

static int read_string(
	struct lxstate *state,
	char const *str,
	char const **end,
	struct lxvalue *val)
{
	size_t cells, free_bytes;
	char const *q;
	char *smin, *smax, *s;
	size_t n;

	q = str;
	cells = alloc_free_count(&state->alloc);
	if (cells > 0) {
		smin = s = (char *)(state->alloc.raw_free + 1);
		/* reserve one cell for length */
		cells--;
	}
	free_bytes = cells * sizeof (union lxcell) - 1;
	smax = smin + free_bytes;
	while (*q != '\"') {
		if (*q == '\0') { return LX_READ_INCOMPLETE; }
		if (s == smax) {
			lx_handle_out_of_memory(state);
		}
		if (*q == '\\') {
			switch (*++q) {
			case '\0': return LX_READ_INCOMPLETE;
			case 't': *s++ = '\t'; break;
			case 'v': *s++ = '\v'; break;
			case 'r': *s++ = '\r'; break;
			case 'n': *s++ = '\n'; break;
			default: *s++ = *q;
			}
			q++;
		} else {
			*s++ = *q++;
		}
	}
	*s = '\0';
	n = s - smin;
	*val = mkref(lx_string_tag, 0, state->alloc.raw_free + 1);
	state->alloc.raw_free->i = s - smin;
	state->alloc.raw_free += ceil_div(n + 1, sizeof (union lxcell)) + 1;

	*end = q + 1;
	return 0;
}

static int read_int(
	char const *str,
	char const **end,
	struct lxvalue *val,
	int base)
{
	intmax_t i;

	/* ignore range errors */
	i = strtoimax(str, (char **)end, base);
	if (*end == str || !is_separator(**end)) {
		return -1;
	}
	*val = lx_valuei(i);
	return 0;
}

static int read_float(
	char const *str,
	char const **end,
	struct lxvalue *val)
{
	double f;

	/* ignore range errors */
	f = strtod(str, (char **)end);
	if (*end == str || !is_separator(**end)) {
		return -1;
	}
	*val = lx_valuef(f);
	return 0;
}

static int read_keyword(
	char const *str,
	char const **end,
	struct lxvalue *val)
{
	if (strncmp(str, "nil", 3) == 0 && is_separator(str[3])) {
		*val = lx_nil();
		*end += strlen("nil");
		return 0;
	}
	return -1;
}

static struct lxlist last(struct lxlist list)
{
	struct lxlist p;
	
	assert(!lx_is_empty_list(list));
	p = list;
	while (!lx_is_empty_list(lx_cdr(p))) {
		p = lx_cdr(p);
	}
	assert(lx_length(p) == 1);
	return p;
}

static int read_atom(
	struct lxstate *s,
	char const *str,
	char const **end,
	struct lxvalue *val)
{
	size_t n;

	assert(!is_separator(*str));

	if (read_int(str, end, val, 10) == 0) {
		return 0;
	}
	if (read_float(str, end, val) == 0) {
		return 0;
	}
	if (read_keyword(str, end, val) == 0) {
		return 0;
	}
	n = strcspn(str, SEPARATORS);
	assert(n > 0);
	*val = lx_strndup(s, str, n).value;
	*end = str + n;
	return 0;
}

struct lxread lx_read(struct lxstate *s, char const *str)
{
	/* The stack is a list of lists, where the list of the top contains the
	   items of the current data structure that's being built. The lists in
	   the stack should also be treated as stacks and a newly parsed value
	   can simply be pushed onto them. They are initialized with an int
	   value (which ends up at the end of the list) that should be
	   interpreted as a tag (lx_list_tag or lx_map_tag). */
	struct lxlist stack, list;
	struct lxmap map;
	struct lxvalue key, val;
	char const *p, *q;
	int err;

	stack = lx_empty_list();
	p = str;

	while (true) {
		p = skip_ws(p);
		switch (*p) {
		case '\0':
			return read_err(LX_READ_INCOMPLETE, str);

		case '(':
			val = lx_empty_list().value;
			q = skip_ws(p + 1);

			if (*q == ')') {
				/* micro optimization: avoid pushing onto the
				   stack if we're parsing an empty list */
				q++;
				break;
			} else {
				/* push new list on top of stack and start
				   parsing the next value */
				list = lx_single(s, lx_valuei(lx_list_tag));
				stack = lx_cons(s, list.value, stack);
				p = q;
				continue;
			}

		case ')':
			if (lx_is_empty_list(stack)) {
				return read_err(LX_READ_UNEXPECTED, p);
			}
			list = lx_list(lx_car(stack));
			list = lx_reverse(s, list);
			if (lx_cari(list) != lx_list_tag) {
				return read_err(LX_READ_UNEXPECTED, p);
			}
			val = lx_cdr(list).value;
			stack = lx_cdr(stack); /* pop stack */
			q = p + 1;
			break;

		case '{':
			q = skip_ws(p + 1);
			if (*q == '}') {
				/* micro optimization: avoid pushing onto the
				   stack if we're parsing an empty map */
				val = lx_empty_map().value;
				q++;
				break;
			} else {
				/* start collecting key-values in a new list on
				   top of the stack */
				list = lx_single(s, lx_valuei(lx_map_tag));
				stack = lx_cons(s, list.value, stack);
				p = q;
				continue;
			}

		case '}':
			if (lx_is_empty_list(stack)) {
				return read_err(LX_READ_UNEXPECTED, p);
			}
			list = lx_list(lx_car(stack));
			if (lx_cari(last(list)) != lx_map_tag) {
				return read_err(LX_READ_UNEXPECTED, p);
			}

			/* list should be of odd length */
			map = lx_empty_map();
			while (true) {
				/* list is reverse of declared values,
				   values/keys */
				val = lx_car(list);
				list = lx_cdr(list);
				if (lx_is_empty_list(list)) {
					/* reached stack frame tag */
					break;
				}
				key = lx_car(list);
				list = lx_cdr(list);
				if (lx_is_empty_list(list)) {
					/* even numbered elements in list: odd
					   number of items in input */
					return read_err(LX_READ_ENTRY, p);
				}
				map = lx_map_set(s, map, key, val);
			}
			val = map.value;
			stack = lx_cdr(stack); /* pop stack */
			q = p + 1;
			break;

		case '#':
			if (strchr("tf", p[1]) && is_separator(p[2])) {
				val = lx_valueb(p[1] == 't');
				q = p + 2;
				break;
			}
			return read_err(LX_READ_SHARP, p);

		case '"':
			err = read_string(s, p + 1, &q, &val);
			if (err) { return read_err(err, p); }
			break;

		default:
			/* atom: nil, string, integer, float */
			err = read_atom(s, p, &q, &val);
			if (err) { return read_err(err, p); }
			break;
		}

		/* a new value has been read; either insert into a list or map,
		   or return a top level value */
		if (lx_is_empty_list(stack)) {
			return read_ok(val, q);
		}

		/* push new value to top-most list */
		list = lx_list(lx_car(stack));
		list = lx_cons(s, val, list);

		/* replace top of stack with new list or map */
		stack = lx_cons(s, list.value, lx_cdr(stack));
		p = q;
	}
}

struct lxread lx_heap_read(struct lxheap *heap, char const *str)
{
	struct lxread read;
	struct lxstate s[1];

	if (lx_start(s, heap) < 0) {
		read.status = LX_READ_HEAP_SIZE;
		read.where = str;
		read.value = lx_valueb(false);
	} else {
		read = lx_read(s, str);
		lx_end(s, read.value);
	}
	return read;
}
