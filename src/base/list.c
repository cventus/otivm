
#include <stddef.h> /* sizeof_t max_align_t */
#include <stdlib.h> /* malloc */
#include <stdio.h>

union node_align
{
	struct link { struct link *prev, *next; } l;
	max_align_t align;
};

typedef void linkfn(struct link *, struct link *);

static void append(struct link *n, struct link *elem)
{
	if ((n->next = elem->next)) { n->next->prev = n; }
	elem->next = n;
	n->prev = elem;
}

static void prepend(struct link *n, struct link *elem)
{
	if ((n->prev = elem->prev)) { n->prev->next = n; }
	elem->prev = n;
	n->next = elem;
}

static struct link *get_link(void *node)
{
	return &((union node_align *)node - 1)->l;
}

static void *get_node(struct link *link)
{
	return (union node_align *)link + 1;
}

/* Create a new node of size `size` and insert it into the same list as `node`
   in the way specified by `link`. */
static void *create_node(void *node, size_t size, linkfn *link)
{
	union node_align *align;
	struct link *p;

	align = malloc(sizeof *align + size);
	if (!align) { return NULL; }
	p = &align->l;
	if (node) {
		link(p, get_link(node));
	} else {
		p->next = p->prev = NULL;
	}
	return align + 1;
}

void *list_head(void *node)
{
	struct link *p;
	if (!node) { return NULL; }
	for (p = get_link(node); p->prev; p = p->prev);
	return get_node(p);
}

void *list_tail(void *node)
{
	struct link *p;
	if (!node) { return NULL; }
	for (p = get_link(node); p->next; p = p->next);
	return get_node(p);
}

void *list_next(void *node)
{
	struct link *p = get_link(node)->next;
	return p ? get_node(p) : NULL;
}

void *list_prev(void *node)
{
	struct link *p = get_link(node)->prev;
	return p ? get_node(p) : NULL;
}

/* insert a new member of the given size in the list after `node` */
void *list_append(void *node, size_t size)
{
	return create_node(node, size, append);
}

/* insert a new member of the given size in the list before `node` */
void *list_prepend(void *node, size_t size)
{
	return create_node(node, size, prepend);
}

void *list_remove(void *node)
{
	struct link *p;
	void *res;

	res = NULL;
	p = get_link(node);
	if (p->next) {
		p->next->prev = p->prev;
		res = get_node(p->next);
	}
	if (p->prev) {
		p->prev->next = p->next;
		if (!res) { res = get_node(p->prev); }
	}
	free(p); /* the link `p` points to beginning of the allocated object */
	return res;
}

void list_free(void *node)
{
	void *p;
	for (p = node; p; p = list_remove(p)) { }
}

size_t list_length(void *node)
{
	struct link *p, *q;
	size_t n;

	if (!node) { return 0; }
	n = 1;
	p = get_link(node);
	for (q = p->prev; q; q = q->prev) { n++; }
	for (q = p->next; q; q = q->next) { n++; }
	return n;
}

