#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "str.h"
#include "list.h"

union lxvalue lx_car(struct lxlist list)
{
	switch (list_car_tag(list)) {
	case lx_list_tag:
		if (isnilref(list_car(list))) {
			return lx_list(lx_empty_list());
		} else {
			return lx_list(deref_list(list_car(list)));
		}
	case lx_string_tag: return ref_to_string(deref_string(list_car(list)));
	case lx_bool_tag: return lx_bool(list_car(list)->i);
	case lx_int_tag: return lx_int(list_car(list)->i);
	case lx_float_tag:
#ifdef lxfloat
		return lx_float(list_car(list)->f);
#endif
	default: abort();
	}
}

struct lxlist lx_cdr(struct lxlist list)
{
	switch (list_cdr_code(list)) {
	case cdr_nil: return lx_empty_list();
	case cdr_link: return deref_list(ref_data(list_forward(list).ref));
	case cdr_adjacent: return list_forward(list);
	default: abort();
	}
}

struct lxlist lx_drop(struct lxlist list, lxint i)
{
	struct lxlist p = list;
	lxint j = 0;

	while (j < i && p.ref.cell) {
		j++;
		p = lx_cdr(p);
	}
	return p;
}

union lxvalue lx_nth(struct lxlist list, lxint i)
{
	return lx_car(lx_drop(list, i));
}

lxint lx_length(struct lxlist list)
{
	struct lxlist p = list;
	lxint sz = 0;

	while (p.ref.cell) {
		sz += 1;
		p = lx_cdr(p);
	}
	return sz;
}
