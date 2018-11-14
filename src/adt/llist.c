#include <assert.h>
#include <stddef.h>
#include "adt/ilist.h"

void llist_init(struct ilist *list)
{
	assert(list != NULL);
	list->prev = list->next = NULL;
}

int llist_singleton(struct ilist const *list)
{
	assert(list != NULL);
	return list->prev == NULL && list->next == NULL;
}

void llist_insert_next(struct ilist *list, struct ilist *node)
{
	struct ilist *tail;
	assert(list != NULL);
	assert(node != NULL);

	tail = list->next;

	/* Set up node */
	node->prev = list;
	node->next = tail;

	/* Change list */
	if (tail) { tail->prev = node; }
	list->next = node;
}

void llist_insert_prev(struct ilist *list, struct ilist *node)
{
	struct ilist *head;
	assert(list != NULL);
	assert(node != NULL);

	head = list->prev;

	/* Set up node */
	node->prev = head;
	node->next = list;

	/* Update list */
	if (head) { head->next = node; }
	list->prev = node;
}

void llist_swap(struct ilist *a, struct ilist *b)
{
	struct ilist *p, *q;

	assert(a != NULL);
	assert(b != NULL);

	if (a->next == b || b->next == a) {
		if (a->next == b) {
			p = a;
			q = b;
		} else {
			p = b;
			q = a;
		}
		if (p->prev) { p->prev->next = q; }
		if (q->next) { q->next->prev = p; }
		q->prev = p->prev;
		p->next = q->next;
		p->prev = q;
		q->next = p;
	} else if (a != b) {
		if (a->prev) { a->prev->next = b; }
		if (a->next) { a->next->prev = b; }
		if (b->prev) { b->prev->next = a; }
		if (b->next) { b->next->prev = a; }

		p = a->next;
		q = a->prev;
		a->next = b->next;
		a->prev = b->prev;
		b->next = p;
		b->prev = q;
	}
}

void llist_remove(struct ilist *node)
{
	struct ilist *head, *tail;
	assert(node != NULL);
	head = node->prev;
	tail = node->next;
	if (head) { head->next = tail; }
	if (tail) { tail->prev = head; }
}

size_t llist_length(struct ilist *node)
{
	struct ilist *p;
	size_t n;
	for (p = node, n = 0; p; p = p->next, n++) {}
	return n;
}
