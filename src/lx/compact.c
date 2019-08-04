#include <stdarg.h>
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
#include "str.h"
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

static struct lxref copy_string(struct lxref string, struct lxalloc *to)
{
	struct lxref copy;
	union lxcell *src, *dest;
	lxint length = string.cell->i;

	if (length < 0) {
		/* already copied */
		copy = dexref(string.cell + 1, to->min_addr, lx_string_tag);
	} else {
		/* move string allocation pointer backwards */
		to->raw_free -= ceil_div(length + 1, CELL_SIZE) + 1;
		src = (union lxcell *)string.cell;
		dest = to->raw_free;
		copy = mkref(lx_string_tag, 0, dest);

		/* copy string into to-space */
		dest->i = length;
		memcpy(dest + 1, src + 1, length + 1);

		/* store forward cross refereence in source string */
		src->i = -1;
		setxref(src + 1, to->min_addr, copy);
	}
	return copy;
}

static void copy_car(
	struct lxref dest,
	struct lxlist list,
	union lxcell *from,
	struct lxalloc *to)
{
	struct lxref car;
	switch (list_car_tag(list)) {
	default: abort();
	case lx_string_tag:
		car = deref(list_car(list), lx_string_tag);
		setref(ref_data(dest), copy_string(car, to));
		break;
	case lx_list_tag:
		if (!isnilref(list_car(list))) {
			/* Copy cross reference into to-space which will be
			   upated as the "scan" pointer advances. */
			car = deref(list_car(list), lx_list_tag);
			setxref(ref_data(dest), from, car);
			break;
		}
		/* else fall-through */
	case lx_bool_tag:
	case lx_int_tag:
	case lx_float_tag:
		/* Immediate values can be copied right away */
		*ref_data(dest) = *list_car(list);
		break;
	}
}

static void adjust_segment_lengths(struct lxref ref)
{
	size_t i;
	lxtag tag;

	for (i = 2; i < MAX_SEGMENT_LENGTH; i++) {
		ref = backward(ref);
		tag = *ref_tag(ref);
		if (lxtag_len(tag) != MAX_SEGMENT_LENGTH) {
			break;
		}
		*ref_tag(ref) = mktag(i, lxtag_tag(tag));
	}
}

/* Shallowly copy list, which is in from-space, into to-space and leave a
   forward-pointer (the copy's offset in to-space) in the CAR. */
static struct lxref copy_list(
	struct lxlist list,
	union lxcell *from,
	struct lxalloc *to,
	void *bitset)
{
	struct lxref dest, result;
	struct lxlist src;
	size_t i;
	enum lx_tag tag;
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
		copy_car(dest, src, from, to);
		if (shared) {
			/* If this list segment is shared with other data
			   structures then it must not be copied multiple
			   times. By clearing the bits the cell is marked as
			   moved and a forwarding pointer back to to-space is
			   stored in from-space. */
			clear_bits(bitset, i);
			setxref(set_car(src), to->min_addr, dest);
		}
		/* The tag depends on the cdr code. If possible, compact a link
		   into an adjacent cell. Then advance src. */
		switch (list_cdr_code(src)) {
		default: abort();
		case cdr_nil:
			/* entire list copied */
			adjust_segment_lengths(dest);
			*ref_tag(dest) = mktag(1, tag);
			to->tag_free = forward(dest);
			return result;
		case cdr_link:
			src = deref_list(list_car(list_forward(src)));
			i = ref_offset(from, src.ref);
			if (is_shared(bitset, i)) {
				/* Shared structure should not be compacted. */
				adjust_segment_lengths(dest);
				*ref_tag(dest) = mktag(0, tag);
				dest = forward(dest);
				/* Leave forward reference to it and stop. */
				*ref_tag(dest) = mktag(1, lx_list_tag);
				setxref(ref_data(dest), from, src.ref);
				to->tag_free = forward(dest);
				return result;
			}
			break;
		case cdr_adjacent:
			src = list_forward(src);
			i = ref_offset(from, src.ref);
			break;
		}
		*ref_tag(dest) = mktag(MAX_SEGMENT_LENGTH, tag);
		dest = forward(dest);
	}
}

/* Adapted from: Cheney, C. J. (November 1970). "A Nonrecursive List Compacting
   Algorithm", Communications of the ACM. 13 (11): 677-678. */
static struct lxlist cheney70(
	struct lxlist root,
	union lxcell *from,
	struct lxalloc *to,
	void *bitset)
{
	size_t i;
	union lxcell *from_car, *to_car;
	struct lxref ref, scan;
	struct lxlist result, from_list;

	/* allocate one cell as a sentinel value when scanning backwards */
	*ref_tag(to->tag_free) = mktag(1, lx_int_tag);
	ref_data(to->tag_free)->i = 0;
	to->tag_free = forward(to->tag_free);

	/* copy_list places the list at the start */
	scan = copy_list(root, from, to, bitset);
	result = ref_to_list(scan);
	result.ref.tag = lx_list_tag;

	/* update references until the free pointer has been reached */
	while (ref_lt(scan, to->tag_free)) {
		switch (lxtag_tag(*ref_tag(scan))) {
		default: abort();
		case lx_list_tag:
			if (isnilref(ref_data(scan))) {
				/* don't copy empty list */
				break;
			}
			/* Data of element contains cross-reference to source
			   in from-space */
			to_car = ref_data(scan);
			ref = dexref(to_car, from, lx_list_tag);
			i = ref_offset(from, ref);
			if (is_forwarded(bitset, i)) {
				/* Get pointer to already copied list from
				   from-apace */
				from_car = ref_data(ref);
				ref = dexref(from_car, to->min_addr, lx_list_tag);
			} else {
				from_list = ref_to_list(ref);
				ref = copy_list(from_list, from, to, bitset);
			}
			setref(to_car, ref);
			break;
		case lx_string_tag:
		case lx_bool_tag:
		case lx_int_tag:
		case lx_float_tag:
			/* atoms have already been copied */
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
	union lxcell *from,
	struct lxalloc *to,
	void *bitset,
	size_t bitset_size)
{
	switch (root.tag) {
	default: abort();
	case lx_bool_tag:
	case lx_int_tag:
	case lx_float_tag:
		return root;

	case lx_string_tag:
		return ref_to_string(copy_string(string_to_ref(root), to));

	case lx_list_tag:
		if (lx_is_empty_list(root.list)) {
			return root;
		}
		memset(bitset, 0, bitset_size);
		/* use to-space as a stack while marking */
		lx_count_refs(root, from, to->max_addr, bitset);
		return lx_list(cheney70(root.list, from, to, bitset));
	}
}
