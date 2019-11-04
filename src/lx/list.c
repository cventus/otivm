#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>
#include <setjmp.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "str.h"
#include "list.h"
#include "tree.h"

struct lxvalue lx_car(struct lxlist list)
{
	switch (list_car_tag(list)) {
	default: abort();
	case lx_list_tag: return deref_list(list_car(list)).value;
	case lx_tree_tag: return deref_tree(list_car(list)).value;
	case lx_string_tag: return deref_string(list_car(list)).value;
	case lx_bool_tag: return lx_valueb(list_car(list)->i);
	case lx_int_tag: return lx_valuei(list_car(list)->i);
	case lx_float_tag: return lx_valuef(list_car(list)->f);
	}
}

bool lx_carb(struct lxlist list) { return list_car(list)->i; }
lxint lx_cari(struct lxlist list) { return list_car(list)->i; }
lxfloat lx_carf(struct lxlist list) { return list_car(list)->f; }

struct lxlist lx_cdr(struct lxlist list)
{
	switch (list_cdr_code(list)) {
	case cdr_nil: return lx_empty_list();
	case cdr_link: return deref_list(ref_data(forward(list.value)));
	case cdr_adjacent: return ref_to_list(forward(list.value));
	default: abort();
	}
}

struct lxlist lx_drop(struct lxlist list, lxint i)
{
	struct lxvalue ref;
	size_t raw_len, len;

	if (i <= 0) {
		return list;
	}
	ref = list.value;
	while (!ref_is_nil(ref)) {
		raw_len = lxtag_len(*ref_tag(ref));
		len = raw_len == 0 ? 1 : raw_len;

		if ((size_t)i < len) {
			ref = ref_advance(ref, i);
			break;
		} else {
			ref = ref_advance(ref, len - 1);
			switch (lxtag_len(*ref_tag(ref))) {
			case 0:
				/* Follow link to next segment */
				ref = deref_list(ref_data(forward(ref))).value;
				i -= len;
				break;
			case 1:
				/* Offset past length of list, return empty. */
				return lx_empty_list();
			default:
				/* Sequence length saturated, continue. */
				i -= len;
				break;
			}
		}
	}
	return ref_to_list(ref);
}

struct lxvalue lx_nth(struct lxlist l, lxint i)
{
	return lx_car(lx_drop(l, i));
}

bool lx_nthb(struct lxlist l, lxint i) { return lx_carb(lx_drop(l, i)); }
lxint lx_nthi(struct lxlist l, lxint i) { return lx_cari(lx_drop(l, i)); }
lxfloat lx_nthf(struct lxlist l, lxint i) { return lx_carf(lx_drop(l, i)); }

lxint lx_length(struct lxlist list)
{
	struct lxlist p = list;
	lxint sz = 0;

	while (!lx_is_empty_list(p)) {
		sz += 1;
		p = lx_cdr(p);
	}
	return sz;
}

struct lxlist lx_cons(
	struct lxstate *s,
	struct lxvalue val,
	struct lxlist list)
{
	struct lxvalue ref, cdr;
	size_t len;

	if (lx_is_empty_list(list)) {
		/* singleton */
		len = 1;
	} else if (ref_eq(list.value, s->alloc.tag_free)) {
		/* create compact list */
		len = lxtag_len(*ref_tag(list.value));
		if (len == 0) {
			len = 2;
		} else if (len < MAX_SEGMENT_LENGTH) {
			len++;
		}
	} else {
		/* normal cons with a link reference */
		len = 0;
	}
	if (lx_reserve_tagged(&s->alloc, len == 0 ? 2 : 1, &ref)) {
		longjmp(s->escape, s->oom);
	}
	*ref_tag(ref) = mktag(len, val.tag);
	lx_set_cell_data(ref_data(ref), val);
	if (len == 0) {
		cdr = forward(ref);
		*ref_tag(cdr) = mktag(1, lx_list_tag);
		setref(ref_data(cdr), list.value);
	}
	ref.tag = lx_list_tag;
	return ref_to_list(ref);
}

struct lxlist lx_reverse(struct lxstate *s, struct lxlist list)
{
	struct lxlist res, l;

	l = list;
	res = lx_empty_list();
	while (!lx_is_empty_list(l)) {
		res = lx_cons(s, lx_car(l), res);
		l = lx_cdr(l);
	}
	return res;
}

struct lxlist lx_single(struct lxstate *s, struct lxvalue a)
{
	return lx_cons(s, a, lx_empty_list());
}

struct lxlist lx_pair(struct lxstate *s, struct lxvalue a, struct lxvalue b)
{
	return lx_cons(s, a, lx_single(s, b));
}

struct lxlist lx_triple(
	struct lxstate *s,
	struct lxvalue a,
	struct lxvalue b,
	struct lxvalue c)
{
	return lx_cons(s, a, lx_pair(s, b, c));
}
