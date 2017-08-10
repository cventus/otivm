#include <stddef.h>
#include <assert.h>

#include <base/mem.h>
#include "include/ilist.h"
#include "include/itree.h"

static struct itree *to_sibling(struct ilist *sibling)
{
	return container_of(sibling, struct itree, siblings);
}

static struct itree *to_parent(struct ilist *parent)
{
	return container_of(parent, struct itree, children);
}

void itree_init(struct itree *node)
{
	assert(node != NULL);

	node->parent = NULL;

	/* siblings is a circular list when there's a parent, but (singleton)
	   linear list otherwise */
	llist_init(&node->siblings);

	/* children is always a circular list */
	clist_init(&node->children);
}

struct itree *itree_parent(struct itree *node)
{
	return node && node->parent ? to_parent(node->parent) : NULL;
}


struct itree *itree_prune(struct itree *node)
{
	struct itree *sibling;
	assert(node != NULL);
	assert(node->parent != NULL);
	assert(!llist_singleton(&node->siblings));

	sibling = itree_next_sibling(node);
	node->parent = NULL;
	clist_remove(&node->siblings);
	llist_init(&node->siblings);
	return sibling;
}

void itree_graft(struct itree *stock, struct itree *scion, struct itree *next)
{
	struct ilist *pos;
	assert(stock != NULL);
	assert(scion != NULL);
	assert(scion->parent == NULL);
	assert(llist_singleton(&scion->siblings));
	if (next) {
		assert(itree_parent(next) == stock);
		pos = &next->siblings;
	} else {
		pos = &stock->children;
	}
	clist_insert_prev(pos, &scion->siblings);

	/* point to the list node so we can identify the end of the sibling
	   list later, but also derive pointer to parent */
	scion->parent = &stock->children;
}

size_t itree_depth(struct itree const *node)
{
	struct itree *p;
	size_t i;

	assert(node != NULL);
	
	i = 0;
	p = (struct itree *)node;
	while (p = itree_parent(p), p != NULL) i++;
	return i;
}

size_t itree_child_count(struct itree const *node)
{
	struct itree *p;
	size_t i;

	assert(node != NULL);

	i = 0;
	p = (struct itree *)node;
	for (p = itree_first_child(p); p; p = itree_next_sibling(p)) i++;
	return i;
}

struct itree *itree_first_child(struct itree *node)
{
	assert(node != NULL);
	return clist_singleton(&node->children)
		? NULL
		: to_sibling(node->children.next);
}

struct itree *itree_last_child(struct itree *node)
{
	assert(node != NULL);
	return clist_singleton(&node->children)
		? NULL
		: to_sibling(node->children.prev);
}

struct itree *itree_next_sibling(struct itree *node)
{
	assert(node != NULL);
	return (node->siblings.next == node->parent)
		? NULL
		: to_sibling(node->siblings.next);
}

struct itree *itree_prev_sibling(struct itree *node)
{
	assert(node != NULL);
	return (node->siblings.prev == node->parent)
		? NULL
		: to_sibling(node->siblings.prev);
}
