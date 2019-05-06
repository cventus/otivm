#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "list.h"

bool lx_equals(union lxvalue a, union lxvalue b)
{
	struct lxlist p, q;

	switch (a.tag) {
	case lx_list_tag:
		if (b.tag != lx_list_tag) { return false; }
		p = a.list;
		q = b.list;
		if (list_eq(p, q)) { return true; }
		if (!p.ref.cell || !q.ref.cell) { return false; }
		do {
			if (!lx_equals(lx_car(p), lx_car(q))) {
				return false;
			}
			p = lx_cdr(p);
			q = lx_cdr(q);
		} while (p.ref.cell && q.ref.cell);
		return list_eq(p, q);
	case lx_string_tag:
		if (b.tag != lx_string_tag) { return false; }
		return a.s == b.s || strcmp(a.s, b.s) == 0;
	case lx_bool_tag:
		if (b.tag != lx_bool_tag) { return false; }
		return a.b == b.b;
	case lx_int_tag:
		switch (b.tag) {
		case lx_int_tag: return a.i == b.i;
		case lx_float_tag: return (lxfloat)a.i == b.f;
		default: return false;
		}
	case lx_float_tag:
#ifdef lxfloat
		switch (b.tag) {
		case lx_int_tag: return a.f == (lxfloat)b.i;
		case lx_float_tag: return a.f == b.f;
		default: return false;
		}
#endif
	default: return false;
	}
}
