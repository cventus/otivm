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
#include "str.h"
#include "list.h"
#include "tree.h"

static struct lxread read_ok(union lxvalue result, char const *where) {
	return (struct lxread) { LX_READ_OK, where, result };
}

static struct lxread read_err(enum lx_read_error err, char const *where) {
	return (struct lxread) { err, where, lx_bool(0) };
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
	struct lxmem *mem,
	char const *str,
	char const **end,
	union lxvalue *val)
{
	size_t cells, free_bytes;
	char const *q;
	char *smin, *smax, *s;
	size_t n;

	q = str;
	cells = alloc_free_count(&mem->alloc);
	if (cells > 0) {
		smin = s = (char *)(mem->alloc.raw_free + 1);
		/* reserve one cell for length */
		cells--;
	}
	free_bytes = cells * sizeof (union lxcell) - 1;
	smax = smin + free_bytes;
	while (*q != '\"') {
		if (*q == '\0') { return LX_READ_INCOMPLETE; }
		if (s == smax) {
			longjmp(mem->escape, mem->oom);
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
	mem->alloc.raw_free->i = s - smin;
	mem->alloc.raw_free += ceil_div(n + 1, sizeof (union lxcell)) + 1;

	val->tag = lx_string_tag;
	val->s = smin;
	*end = q + 1;
	return 0;
}

static int read_int(
	char const *str,
	char const **end,
	union lxvalue *val,
	int base)
{
	intmax_t i;

	/* ignore range errors */
	i = strtoimax(str, (char **)end, base);
	if (*end == str || !is_separator(**end)) {
		return -1;
	}
	*val = lx_int(i);
	return 0;
}

static int read_float(
	char const *str,
	char const **end,
	union lxvalue *val)
{
	double f;

	/* ignore range errors */
	f = strtod(str, (char **)end);
	if (*end == str || !is_separator(**end)) {
		return -1;
	}
	*val = lx_float(f);
	return 0;
}

static int read_atom(
	struct lxmem *mem,
	char const *str,
	char const **end,
	union lxvalue *val)
{
	size_t n;

	assert(!is_separator(*str));
	if (read_int(str, end, val, 10) && read_float(str, end, val)) {
		n = strcspn(str, SEPARATORS);
		assert(n > 0);
		*val = lx_strndup(mem, str, n);
		*end = str + n;
		return 0;
	}
	return 0;
}

struct lxread lx_read(struct lxmem *mem, char const *str)
{
	char const *p, *q;
	union lxvalue val, top;
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
			val = lx_list(lx_empty_list());
			q = skip_ws(p + 1);

			if (*q == ')') {
				/* micro optimization: avoid pushing onto the
				   stack if we're parsing an empty list */
				q++;
				break;
			} else {
				/* push new list on top of stack and start
				   parsing the next value */
				stack = lx_cons(mem, val, stack);
				p = q;
				continue;
			}

		case '{':
			val = lx_tree(lx_empty_tree());
			q = skip_ws(p + 1);

			if (*q == '}') {
				/* micro optimization: avoid pushing onto the
				   stack if we're parsing an empty tree */
				q++;
				break;
			} else {
				/* push new tree on top of stack and start
				   parsing the first entry */
				stack = lx_cons(mem, val, stack);
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
				val = lx_list(lx_reverse(mem, val.list));
			}
			stack = lx_cdr(stack); /* pop stack */
			q = p + 1;
			break;

		case '#':
			if (strchr("tf", p[1]) && is_separator(p[2])) {
				val = lx_bool(p[1] == 't');
				q = p + 2;
				break;
			}
			return read_err(LX_READ_SHARP, p);

		case '"':
			err = read_string(mem, p + 1, &q, &val);
			if (err) { return read_err(err, p); }
			break;

		default:
			/* atom: string, integer, float */
			err = read_atom(mem, p, &q, &val);
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
			top.list = lx_cons(mem, val, top.list);
		} else {
			assert(top.tag == lx_tree_tag);
			if (val.tag != lx_list_tag) {
				return read_err(LX_READ_ENTRY, p);
			}
			if (lx_is_empty_list(top.list)) {
				return read_err(LX_READ_ENTRY, p);
			}
			top.tree = lx_tree_cons(mem, val.list, top.tree);
		}
		stack = lx_cons(mem, top, lx_cdr(stack));
		p = q;
	}
}
