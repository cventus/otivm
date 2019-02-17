#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <limits.h>
#include <string.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "list.h"
#include "mark.h"

static union lxcell *set_car(struct lxlist list)
{
	return (union lxcell *)list_car(list);
}

static bool is_forwarded(void *bitset, size_t i)
{
	return get_bits(bitset, i) == 0;
}

static bool is_shared(void *bitset, size_t i)
{
	return get_bits(bitset, i) != 0x1;
}

static void copy_car(
	struct lxref dest,
	struct lxlist list,
	struct lxspace *from)
{
	struct lxref car;
	switch (list_car_tag(list)) {
	case lx_list_tag:
		/* Copy cross reference into to-space which will be upated as
		   the "scan" pointer advances. */
		car = deref(list_car(list), lx_list_tag);
		setxref(ref_data(dest), from->begin, car);
		break;
	case lx_nil_tag:
	case lx_bool_tag:
	case lx_int_tag:
	case lx_float_tag:
		/* Immediate values can be copied right away */
		*ref_data(dest) = *list_car(list);
		break;
	default:
		abort();
	}
}

/* Shallowly copy list, which is in from-space, into to-space and leave a
   forward-pointer (the copy's offset in to-space) in the CAR. */
static struct lxref copy_list(
	struct lxlist list,
	struct lxspace *from,
	struct lxspace *to,
	void *bitset)
{
	struct lxref dest, result;
	struct lxlist src;
	size_t i;
	lxtag tag;
	bool shared;
	lxint diff;

	assert(!is_forwarded(bitset, ref_offset(from, list.ref)));

	i = ref_offset(from, list.ref);
	shared = is_shared(bitset, i);
	dest = to->tag_free;
	if (shared) {
		/* copy the whole list segment when it is shared */
		src = lx_shared_head(list, from, bitset);
		diff = ref_diff(list.ref, src.ref); /* src <= list */
		result = ref_advance(dest, diff);
		i = ref_offset(from, src.ref);
	} else {
		src = list;
		result = dest;
	}
	while (true) {
		tag = list_car_tag(src);
		copy_car(dest, src, from);
		if (shared) {
			/* If this list segment is shared with other data
			   structures then it must not be copied multiple
			   times. By clearing the bits the cell is marked as
			   moved and a forwarding pointer back to to-space is
			   stored in from-space. */
			clear_bits(bitset, i);
			setxref(set_car(src), to->begin, dest);
		}
		/* The tag depends on the cdr code. If possible,
		   compact a link into an adjacent cell. Then advance src. */
		switch (list_cdr_code(src)) {
		case cdr_nil:
			/* entire list copied */
			*ref_tag(dest) = mktag(cdr_nil, tag);
			to->tag_free = forward(dest);
			return result;
		case cdr_link:
			src = deref_list(list_car(list_forward(src)));
			i = ref_offset(from, src.ref);
			if (is_shared(bitset, i)) {
				/* Shared structure should not be compacted. */
				*ref_tag(dest) = mktag(cdr_link, tag);
				dest = forward(dest);
				/* Leave forward reference to it and stop. */
				*ref_tag(dest) = mktag(cdr_nil, lx_list_tag);
				setxref(ref_data(dest), from->begin, src.ref);
				to->tag_free = forward(dest);
				return result;
			}
			break;
		case cdr_adjacent:
			src = list_forward(src);
			i++;
			break;
		default:
			abort();
		}
		*ref_tag(dest) = mktag(cdr_adjacent, tag);
		dest = forward(dest);
	}
}

/* Adapted from: Cheney, C. J. (November 1970). "A Nonrecursive List Compacting
   Algorithm", Communications of the ACM. 13 (11): 677-678. */
static struct lxlist cheney70(
	struct lxlist root,
	struct lxspace *from,
	struct lxspace *to,
	void *bitset)
{
	size_t i;
	union lxcell *from_car, *to_car;
	struct lxref ref, scan;
	struct lxlist result, from_list;

	/* copy_list places the list at the start */
	scan = copy_list(root, from, to, bitset);
	result = ref_to_list(scan);
	result.ref.tag = lx_list_tag;

	/* update references until the free pointer has been reached */
	while (ref_lt(scan, to->tag_free)) {
		switch (lxtag_tag(*ref_tag(scan))) {
		case lx_list_tag:
			/* Data of element contains cross-reference to source
			   in from-space */
			to_car = ref_data(scan);
			ref = dexref(to_car, from->begin, lx_list_tag);
			i = ref_offset(from, ref);
			if (is_forwarded(bitset, i)) {
				/* Get pointer to already copied list from
				   from-apace */
				from_car = ref_data(ref);
				ref = dexref(from_car, to->begin, lx_list_tag);
			} else {
				from_list = ref_to_list(ref);
				ref = copy_list(from_list, from, to, bitset);
			}
			setref(to_car, ref);
			break;
		case lx_nil_tag:
		case lx_bool_tag:
		case lx_int_tag:
		case lx_float_tag:
			/* small data has already been copied */
			break;
		default:
			abort();
			break;
		}
		scan = forward(scan);
	}
	return result;
}

/* Copy and compact the root value (which resides in from-space) into to-space
   by traversing the live-set in two passes. First, mark all reachable nodes in
   order to find lists which are shared and which only have one reference (i.e.
   can be compacted through cdr-coding) and use to-space as a stack. Second,
   copy lists breadth-first using Cheney's algorithm. */
union lxvalue lx_compact(
	union lxvalue root,
	struct lxspace *from,
	struct lxspace *to)
{
	size_t mark_cells, raw_cells;
	union lxcell *bitset;

	assert(is_tospace(to));
	assert(space_size(to) >= space_used(from));

	/* Allocate the bitset for marking shared structure at the end of
	   to-space. */
	mark_cells = mark_cell_count(space_tagged_cells(from));
	raw_cells = space_raw_cells(from);
	bitset = to->end - (mark_cells + raw_cells);
	memset(bitset, 0, mark_cells * sizeof *bitset);

	/* Use area below bitset in to-space as a stack during for marking */
	lx_count_refs(root, from, bitset, bitset);

	switch (root.tag) {
	case lx_list_tag:
		/* Non-trivial case: copy a list structure */
		assert(get_bits(bitset, ref_offset(from, root.list.ref)) == 1);
		return lx_list(cheney70(root.list, from, to, bitset));

	case lx_nil_tag:
	case lx_bool_tag:
	case lx_int_tag:
	case lx_float_tag:
		return root;

	default:
		abort();
	}
}
