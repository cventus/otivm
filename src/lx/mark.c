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
#include "list.h"
#include "tree.h"
#include "mark.h"

static bool is_ref(enum lx_tag tag)
{
	return tag == lx_list_tag || tag == lx_tree_tag;
}

static void push_ref(
	struct lxvalue ref,
	union lxcell const *from,
	union lxcell **stack)
{
	if (ref.tag == lx_tree_tag && !is_leaf_node(ref_to_tree(ref))) {
		ref = backward(backward(ref));
	}
	setxref(--(*stack), from, ref);
}

static struct lxvalue pop_ref(union lxcell **stack, union lxcell const *from)
{
	return dexref((*stack)++, from, lx_list_tag);
}

static void mark_shared_head(
	struct lxvalue segment,
	union lxcell const *from,
	void *bitset)
{
	lxint i;
	struct lxvalue ref;

	ref = lx_shared_head(segment, from, bitset);
	do {
		i = ref_offset(from, ref);
		if (mark_shared_bits(bitset, i) == 0x3) {
			/* rest of list has already been marked as
			   shared, nothing more to do */
			break;
		}
		if (lxtag_cdr(*ref_tag(ref)) == cdr_nil) { break; }
		ref = forward(ref);
	} while (true);
}

/* trace backwards from `list` to the first cell in the same segment of
   adjacent list cells that is marked */
struct lxvalue lx_shared_head(
	struct lxvalue list,
	union lxcell const *from,
	void const *bitset)
{
	struct lxvalue p, q;

	p = list;
	do {
		q = backward(p);

		/* previous is not the end of a preceding list segment */
		if (lxtag_len(*ref_tag(q)) <= 1) { break; }

		/* is referenced by something */
		if (get_bits(bitset, ref_offset(from, q)) == 0) { break; }

		p = q;
	} while (true);

	return p;
}

/* Recursively mark reachable nodes to find shared list structure. */
void lx_count_refs(
	struct lxvalue root,
	union lxcell const *from,
	union lxcell *stack_max,
	void *bitset)
{
	union lxcell *stack, *d;
	struct lxvalue ref, head;
	lxuint i;
	lxtag meta;
	enum lx_tag tag;

	if (!is_ref(root.tag) || ref_is_nil(root)) { return; }

	stack = stack_max;
	push_ref(root, from, &stack);

	while (stack < stack_max) {
		ref = head = pop_ref(&stack, from);
		do {
			i = ref_offset(from, ref);
			if (mark_bits(bitset, i) != 0) {
				/* We found a cell that was visited before.
				   Restart marking and mark whole segment as
				   shared (both bits set). */
				mark_shared_head(head, from, bitset);
				break;
			}
			meta = *ref_tag(ref);

			/* New potential list reference found. A reference is
			   pushed to the stack at most once. */
			tag = lxtag_tag(meta);
			if (is_ref(tag) && !isnilref(d = ref_data(ref))) {
				push_ref(deref(d, tag), from, &stack);
			}

			/* All lists end in cdr_nil. Many segments have the
			   cdr-code `cdr_link` in the second to last tagged
			   union cell. However, the following link cell always
			   have the cdr_nil tag. */
			if (lxtag_cdr(meta) == cdr_nil) {
				break;
			}

			/* Traverse only within segment using forward()
			   instead of lx_cdr(). */
			ref = forward(ref);
		} while (true);
	}
}
