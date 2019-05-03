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
#include "mark.h"

static void push_ref(
	union lxvalue val,
	union lxcell const *from,
	union lxcell **stack)
{
	if (val.tag == lx_list_tag) {
		setxref(--(*stack), from, val.list.ref);
	}
}

static struct lxref pop_ref(union lxcell **stack, union lxcell const *from)
{
	return dexref((*stack)++, from, lx_list_tag);
}

static void mark_shared_list(
	struct lxlist list,
	union lxcell const *from,
	void *bitset)
{
	lxint i;

	list = lx_shared_head(list, from, bitset);
	i = ref_offset(from, list.ref);
	do {
		if (mark_shared_bits(bitset, i) == 0x3) {
			/* rest of list has already been marked as
			   shared, nothing more to do */
			break;
		}
		if (list_cdr_code(list) == cdr_nil) { break; }
		list = list_forward(list);
		i++;
	} while (true);
}

/* trace backwards from `list` to the first cell in the same segment of
   adjacent list cells that is marked */
struct lxlist lx_shared_head(
	struct lxlist list,
	union lxcell const *from,
	void const *bitset)
{
	struct lxref p, q;
	lxint i;

	p = list.ref;
	i = ref_offset(from, p);
	do {
		q = backward(p);
		i--;

		/* previous is not the end of a preceding list segment */
		if (lxtag_len(*ref_tag(q)) <= 1) { break; }

		/* is referenced by something */
		if (get_bits(bitset, i) == 0) { break; }

		p = q;
	} while (true);
	return ref_to_list(p);
}

/* Recursively mark reachable nodes to find shared list structure. */
void lx_count_refs(
	union lxvalue root,
	union lxcell const *from,
	union lxcell *stack_max,
	void *bitset)
{
	union lxcell *stack;
	struct lxlist list, head;
	lxuint i;

	stack = stack_max;
	push_ref(root, from, &stack);

	while (stack < stack_max) {
		list = head = ref_to_list(pop_ref(&stack, from));
		assert(head.tag == lx_list_tag);
		i = ref_offset(from, head.ref);

		do {
			if (mark_bits(bitset, i) != 0) {
				/* We found a cell that was visited before.
				   Restart marking and mark whole segment as
				   shared (both bits set). */
				mark_shared_list(head, from, bitset);
				break;
			}

			/* New potential list reference found. A reference is
			   pushed to the stack at most once. */
			push_ref(lx_car(list), from, &stack);

			/* All list segments end in cdr_nil. Many segments have
			   the cdr-code cdr_link in the second to last tagged
			   union cell. However, the following link cell always
			   have the cdr_nil tag. */
			if (list_cdr_code(list) == cdr_nil) {
				break;
			}

			/* Traverse only within segment using list_forward()
			   instead of lx_cdr(). */
			list = list_forward(list);
			i++;
		} while (true);
	}
}
