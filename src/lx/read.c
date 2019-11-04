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
			longjmp(state->escape, state->oom);
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

static int read_atom(
	struct lxstate *s,
	char const *str,
	char const **end,
	struct lxvalue *val)
{
	size_t n;

	assert(!is_separator(*str));
	if (read_int(str, end, val, 10) && read_float(str, end, val)) {
		n = strcspn(str, SEPARATORS);
		assert(n > 0);
		*val = lx_strndup(s, str, n).value;
		*end = str + n;
	}
	return 0;
}

struct lxread lx_read(struct lxstate *s, char const *str)
{
	char const *p, *q;
	struct lxvalue val, top;
	struct lxlist stack;
	enum lx_tag tag;
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
				stack = lx_cons(s, val, stack);
				p = q;
				continue;
			}

		case '{':
			val = lx_empty_tree().value;
			q = skip_ws(p + 1);

			if (*q == '}') {
				/* micro optimization: avoid pushing onto the
				   stack if we're parsing an empty tree */
				q++;
				break;
			} else {
				/* push new tree on top of stack and start
				   parsing the first entry */
				stack = lx_cons(s, val, stack);
				p = q;
				continue;
			}

		case ')':
		case '}':
			if (lx_is_empty_list(stack)) {
				return read_err(LX_READ_UNEXPECTED, p);
			}
			val = lx_car(stack);
			tag = *p == ')' ? lx_list_tag : lx_tree_tag;
			if (val.tag != tag) {
				return read_err(LX_READ_UNEXPECTED, p);
			}
			if (tag == lx_list_tag) {
				/* lists are built in reverse; reverse the
				   result and get it linearized as a bonus */
				val = lx_reverse(s, lx_list(val)).value;
			}
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
			/* atom: string, integer, float */
			err = read_atom(s, p, &q, &val);
			if (err) { return read_err(err, p); }
			break;
		}

		/* a new value has been read; either insert into a list or
		   tree, or return a top level value */
		if (lx_is_empty_list(stack)) {
			return read_ok(val, q);
		}
		top = lx_car(stack);
		if (top.tag == lx_list_tag) {
			/* build new list in reverse */
			top = lx_cons(s, val, lx_list(top)).value;
		} else {
			assert(top.tag == lx_tree_tag);
			if (val.tag != lx_list_tag) {
				return read_err(LX_READ_ENTRY, p);
			}
			if (lx_is_empty_list(lx_list(val))) {
				return read_err(LX_READ_ENTRY, p);
			}
			top = lx_tree_cons(s,
				lx_list(val),
				lx_tree(top)).value;
		}
		/* replace top of stack with new list or tree */
		stack = lx_cons(s, top, lx_cdr(stack));
		p = q;
	}
}

static struct lxvalue read_it(struct lxstate *s, struct lxvalue val, va_list ap)
{
	char const *str;
	struct lxread *r;

	(void)val;
	r = va_arg(ap, struct lxread *);
	str = va_arg(ap, char const *);

	*r = lx_read(s, str);
	return r->value;
}

struct lxread lx_heap_read(struct lxheap *heap, char const *str)
{
	struct lxread read;

	if (lx_modifyl(heap, read_it, &read, str).status) {
		read.status = LX_READ_HEAP_SIZE;
		read.where = str;
		read.value = lx_valueb(false);
	}
	return read;
}
