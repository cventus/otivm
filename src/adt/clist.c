#include <assert.h>
#include <stddef.h>
#include "include/ilist.h"

void clist_init(struct ilist *list)
{
	assert(list != NULL);
	list->prev = list->next = list;
}

int clist_singleton(struct ilist const *list)
{
	assert(list != NULL);
	return list->next == list;
}

void clist_insert_prev(struct ilist *list, struct ilist *node)
{
	assert(list != NULL);
	assert(node != NULL);

	node->prev = list->prev;
	list->prev->next = node;

	node->next = list;
	list->prev = node;
}

void clist_insert_next(struct ilist *list, struct ilist *node)
{
	assert(list != NULL);
	assert(node != NULL);

	node->next = list->next;
	list->next->prev = node;

	node->prev = list;
	list->next = node;
}

void clist_swap(struct ilist *a, struct ilist *b)
{
	struct ilist *p, *q;

	assert(a != NULL);
	assert(b != NULL);

	if ((a->next == b) != (b->next == a)) {
		if (a->next == b) {
			p = a;
			q = b;
		} else {
			p = b;
			q = a;
		}
		p->prev->next = q;
		q->next->prev = p;
		q->prev = p->prev;
		p->next = q->next;
		p->prev = q;
		q->next = p;
	} else if (a != b && a->next != b && b->next != a) {
		a->prev->next = b;
		a->next->prev = b;
		b->prev->next = a;
		b->next->prev = a;

		p = a->next;
		q = a->prev;
		a->next = b->next;
		a->prev = b->prev;
		b->next = p;
		b->prev = q;
	}
}

void clist_remove(struct ilist *node)
{
	assert(node != NULL);
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

size_t clist_length(struct ilist *list)
{
	struct ilist *p;
	size_t n;
	if (!list) { return 0; }
	for (p = list->next, n = 1; p != list; p = p->next, n++) {}
	return n;
}
