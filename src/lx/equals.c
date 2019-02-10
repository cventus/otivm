#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "list.h"

bool lx_equals(union lxvalue a, union lxvalue b)
{
	struct lxlist p, q;

	if (a.tag != b.tag) {
		return false;
	} else switch (a.tag) {
	case lx_list_tag:
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
	case lx_bool_tag: return a.b == b.b;
	case lx_int_tag: return a.i == b.i;
	case lx_float_tag:
#ifdef lxfloat
		return a.f == b.f;
#endif
	default: return false;
	}
}
