#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "common.h"
#include "lx.h"
#include "memory.h"

enum cdr_code
{
	cdr_nil = 0,
	cdr_link = 1,
	cdr_adjacent = 2
};

/* extract cdr code */
static enum cdr_code lxtag_cdr(lxtag h)
{
	return h >> 6;
}

/* extract type tag */
static enum lx_tag lxtag_tag(lxtag h)
{
	return h & 0x3f;
}

static struct lx_list deref(union lxcell const *c)
{
	return (struct lx_list) {
		lx_list_tag,
		c->i & OFFSET_MASK,
		(union lxcell const*)((char *)c + (c->i & ~OFFSET_MASK))
	};
}

static lxtag list_head(struct lx_list list)
{
	return ((lxtag *)list.cell)[list.offset];
}

static enum cdr_code list_cdr_code(struct lx_list list)
{
	return lxtag_cdr(list_head(list));
}

static enum lx_tag list_car_tag(struct lx_list list)
{
	return lxtag_tag(list_head(list));
}

static union lxcell const *list_car(struct lx_list l)
{
	return (union lxcell const *)l.cell + l.offset + 1;
}

static struct lx_list forward(struct lx_list list)
{
	union lxcell const *newc = list.cell;
	lxint newoff = (list.offset + 1) & OFFSET_MASK;
	if (newoff == 0) {
		newc += SPAN_LENGTH;
	}
	return (struct lx_list) { lx_list_tag, newoff, newc };
}

union lxvalue lx_car(struct lx_list list)
{
	switch (list_car_tag(list)) {
	case lx_list_tag:
		return (union lxvalue) { .list = deref(list_car(list)) };
	case lx_bool_tag: return lx_bool(list_car(list)->i);
	case lx_int_tag: return lx_int(list_car(list)->i);
	case lx_float_tag:
#ifdef lxfloat
		return lx_float(list_car(list)->f);
#endif
	default: abort();
	}
}

struct lx_list lx_cdr(struct lx_list list)
{
	switch (list_cdr_code(list)) {
	case cdr_nil: return (struct lx_list) { lx_list_tag, 0, NULL };
	case cdr_link: return deref(list_car(forward(list)));
	case cdr_adjacent: return forward(list);
	default: abort();
	}
}

struct lx_list lx_drop(struct lx_list list, lxint i)
{
	struct lx_list p = list;
	lxint j = 0;

	while (j < i && p.cell) {
		j++;
		p = lx_cdr(p);
	}
	return p;
}

union lxvalue lx_nth(struct lx_list list, lxint i)
{
	return lx_car(lx_drop(list, i));
}

lxint lx_length(struct lx_list list)
{
	struct lx_list p = list;
	lxint sz = 0;

	while (p.cell) {
		sz += 1;
		p = lx_cdr(p);
	}
	return sz;
}

static bool list_eq(struct lx_list a, struct lx_list b)
{
	return a.cell == b.cell && a.offset == b.offset;
}

bool lx_equals(union lxvalue a, union lxvalue b)
{
	struct lx_list p, q;

	if (a.tag != b.tag) {
		return false;
	} else switch (a.tag) {
	case lx_list_tag:
		p = a.list;
		q = b.list;
		if (list_eq(p, q)) { return true; }
		if (!p.cell || !q.cell) { return false; }
		do {
			if (!lx_equals(lx_car(p), lx_car(q))) {
				return false;
			}
			p = lx_cdr(p);
			q = lx_cdr(q);
		} while (p.cell && q.cell);
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
