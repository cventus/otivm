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
#include "alloc.h"
#include "str.h"
#include "list.h"
#include "tree.h"
#include "mark.h"

static bool is_forwarded(void *bitset, size_t i)
{
	return get_bits(bitset, i) == 0;
}

static bool is_shared(void *bitset, size_t i)
{
	return get_bits(bitset, i) != 0x1;
}

static struct lxvalue copy_string(struct lxvalue string, struct lxalloc *to)
{
	struct lxvalue copy;
	union lxcell *src, *dest;
	lxint length;

	length = lx_strlen(ref_to_string(string));
	if (length < 0) {
		/* already copied */
		copy = dexref(ref_cell(string), to->min_addr, lx_string_tag);
	} else {
		/* move string allocation pointer backwards */
		to->raw_free -= ceil_div(length + 1, CELL_SIZE) + 1;
		src = ref_cell(string);
		dest = to->raw_free;
		copy = mkref(lx_string_tag, 0, dest + 1);

		/* copy string into to-space */
		dest->i = length;
		memcpy(dest + 1, src, length + 1);

		/* store forward cross refereence in source string */
		src[-1].i = -1;
		setxref(src, to->min_addr, copy);
	}
	return copy;
}

static void copy_data(
	struct lxvalue dest,
	struct lxvalue src,
	union lxcell *from,
	struct lxalloc *to)
{
	struct lxvalue car;
	enum lx_tag tag;

	tag = lxtag_tag(*ref_tag(src));
	switch (tag) {
	default: abort();
	case lx_string_tag:
		car = deref(ref_data(src), lx_string_tag);
		setref(ref_data(dest), copy_string(car, to));
		break;
	case lx_list_tag:
	case lx_tree_tag:
		if (!isnilref(ref_data(src))) {
			/* Copy cross reference into to-space which will be
			   upated as the "scan" pointer advances. */
			car = deref(ref_data(src), tag);
			setxref(ref_data(dest), from, car);
			break;
		}
		/* else fall-through */
	case lx_bool_tag:
	case lx_int_tag:
	case lx_float_tag:
		/* Immediate values can be copied right away */
		*ref_data(dest) = *ref_data(src);
		break;
	}
}

static void adjust_segment_lengths(struct lxvalue ref)
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
   forward-pointer (the copy's offset in to-space) in the CAR. `list` is a list
   structure in from-space. Return the copied and possibly compacted list in
   to-space. */
static struct lxvalue copy_list(
	struct lxvalue list,
	union lxcell *from,
	struct lxalloc *to,
	void *bitset)
{
	struct lxvalue src, dest, result;
	lxtag meta;
	enum lx_tag type;
	bool shared;
	lxint diff;

	assert(!is_forwarded(bitset, ref_offset(from, list)));

	shared = is_shared(bitset, ref_offset(from, list));
	dest = to->tag_free;
	dest.tag = list.tag;
	if (shared) {
		/* don't split shared structure; copy common head */
		src = lx_shared_head(list, from, bitset);
		diff = ref_diff(list, src); /* src <= list */
		result = ref_advance(dest, diff);
	} else {
		src = list;
		result = dest;
	}

	/* loop until end of list or a link to a shared segment is reached */
	while (true) {
		meta = *ref_tag(src);
		type = lxtag_tag(meta);
		copy_data(dest, src, from, to);
		if (shared) {
			/* If this list segment is shared with other data
			   structures then it must not be copied multiple
			   times. By clearing the bits the cell is marked as
			   moved and a forwarding pointer back to to-space is
			   stored in from-space. */
			clear_bits(bitset, ref_offset(from, src));
			setxref(ref_data(src), to->min_addr, dest);
		}
		/* The tag depends on the cdr code. If possible, compact a link
		   into an adjacent cell. Then advance src. */
		switch (lxtag_cdr(meta)) {
		default: abort();
		case cdr_nil:
			/* entire list copied */
			adjust_segment_lengths(dest);
			*ref_tag(dest) = mktag(1, type);
			to->tag_free = forward(dest);
			return result;
		case cdr_link:
			src = deref(ref_data(forward(src)), lx_list_tag);
			if (is_shared(bitset, ref_offset(from, src))) {
				/* Shared structure cannot be compacted. */
				adjust_segment_lengths(dest);
				*ref_tag(dest) = mktag(0, type);
				dest = forward(dest);
				/* Leave forward reference to it and stop. */
				*ref_tag(dest) = mktag(1, lx_list_tag);
				setxref(ref_data(dest), from, src);
				to->tag_free = forward(dest);
				return result;
			}
			break;
		case cdr_adjacent:
			src = forward(src);
			break;
		}
		*ref_tag(dest) = mktag(MAX_SEGMENT_LENGTH, type);
		dest = forward(dest);
	}
}

/* Adapted from: Cheney, C. J. (November 1970). "A Nonrecursive List Compacting
   Algorithm", Communications of the ACM. 13 (11): 677-678. */
static struct lxvalue cheney70(
	struct lxvalue root,
	union lxcell *from,
	struct lxalloc *to,
	void *bitset)
{
	size_t i;
	union lxcell *from_data, *to_data;
	struct lxvalue ref, scan;
	enum lx_tag tag;
	struct lxvalue result;

	/* allocate one cell as a sentinel value when scanning backwards in
	   lx_shared_head() - note that the latest allocation in the other end
	   of the heap is never shared */
	*ref_tag(to->tag_free) = mktag(1, lx_int_tag);
	ref_data(to->tag_free)->i = 0;
	to->tag_free = forward(to->tag_free);

	if (root.tag == lx_list_tag || is_leaf_node(lx_tree(root))) {
		scan = copy_list(root, from, to, bitset);
		result = scan;
	} else {
		ref = backward(backward(root));
		scan = copy_list(ref, from, to, bitset);
		result = forward(forward(scan));
	}

	/* update references until the free pointer has been reached */
	while (ref_lt(scan, to->tag_free)) {
		tag = lxtag_tag(*ref_tag(scan));
		switch (tag) {
		default: abort();
		case lx_list_tag:
		case lx_tree_tag:
			if (isnilref(ref_data(scan))) {
				/* don't copy empty list/tree */
				break;
			}
			/* Data of element contains cross-reference to source
			   in from-space */
			to_data = ref_data(scan);
			ref = dexref(to_data, from, tag);
			i = ref_offset(from, ref);
			if (is_forwarded(bitset, i)) {
				/* Get pointer to already copied list from
				   from-apace */
				from_data = ref_data(ref);
				ref = dexref(from_data, to->min_addr, tag);
			} else if (tag == lx_tree_tag && !is_leaf_node(ref_to_tree(ref))) {
				ref = backward(backward(ref));
				ref = copy_list(ref, from, to, bitset);
				ref = forward(forward(ref));
			} else {
				ref = copy_list(ref, from, to, bitset);
			}
			setref(to_data, ref);
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
struct lxvalue lx_compact(
	struct lxvalue root,
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
		return copy_string(root, to);

	case lx_list_tag:
		if (lx_is_empty_list(ref_to_list(root))) { return root; }
		break;

	case lx_tree_tag:
		if (lx_is_empty_tree(ref_to_tree(root))) { return root; }
		break;
	}

	/* tree or list */
	memset(bitset, 0, bitset_size);
	/* use to-space as a stack while marking */
	lx_count_refs(root, from, to->max_addr, bitset);
	return cheney70(root, from, to, bitset);
}
