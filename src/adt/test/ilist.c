#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <ok/ok.h>

#include "../include/ilist.h"

static struct ilist const *const null = NULL;

static int check_linkage(struct ilist const *e0, struct ilist const *e1, ...)
{
	va_list ap;
	struct ilist const *left, *right;
	int i, result;

	left = e0;
	right = e1;

	result = 0;
	i = 1;
	va_start(ap, e1);
	if (e0 == null) { goto e0null; }
	do {
		if (left->next != right) { result = i; break; }
		i++;
e0null:		if (right == null) { break; }
		if (right->prev != left) { result = i; break; }
		if (right == e0) { break; }

		/* move forward */
		left = right;
		right = va_arg(ap, struct ilist const *);
	} while (1);
	va_end(ap);
	return result;
}

static int test_check_fail(void)
{
	struct ilist a, b, c, d;

	a.prev = NULL; a.next = &b;
	b.prev = NULL; b.next = &c;
	c.prev =   &b; c.next = &a;
	d.prev =   &c; d.next = &a;
	if (check_linkage(null, null) != 0) { return -1; }
	if (check_linkage(null, &a, null) != 1) { return -1; }
	if (check_linkage(&a, &b, &c, &a, null) != 2) { return -1; }
	if (check_linkage(null, &a, &b, &c, &a, null) != 2) { return -1; }
	if (check_linkage(&b, &c, &a, &b, null) != 3) { return -1; }
	if (check_linkage(&d, &a, &b, null) != 2) { return -1; }
	return 0;
}

static int linit(void)
{
	struct ilist node;
	llist_init(&node);
	return check_linkage(null, &node, null);
}

static int linsert_next(void)
{
	struct ilist a, b, c;
	llist_init(&a);
	llist_insert_next(&a, &c);
	llist_insert_next(&a, &b);
	return check_linkage(null, &a, &b, &c, null);
}

static int lsingleton(void)
{
	struct ilist a, b;
	llist_init(&a);
	if (!llist_singleton(&a)) { return -1; }
	llist_insert_next(&a, &b);
	if (llist_singleton(&a)) { return -2; }
	if (llist_singleton(&b)) { return -3; }
	return 0;
}

static int linsert_prev(void)
{
	struct ilist a, b, c;
	llist_init(&a);
	llist_insert_prev(&a, &c);
	llist_insert_prev(&a, &b);
	return check_linkage(null, &c, &b, &a, null);
}

static int lcreate_list(void)
{
	struct ilist a, b, c, d, e;
	llist_init(&b);
	llist_insert_next(&b, &d);
	llist_insert_next(&b, &c);
	llist_insert_prev(&b, &a);
	llist_insert_next(&d, &e);
	return check_linkage(null, &a, &b, &c, &d, &e, null);
}

static int llength(void)
{
	struct ilist a, b, c;

	llist_init(&a);
	if (llist_length(&a) != 1) { return -1; }
	llist_insert_next(&a, &b);
	if (llist_length(&a) != 2) { return -1; }
	if (llist_length(&b) != 1) { return -1; }
	llist_insert_next(&b, &c);
	if (llist_length(&a) != 3) { return -1; }
	if (llist_length(&b) != 2) { return -1; }
	if (llist_length(&c) != 1) { return -1; }
	return 0;
}

static int lswap_order(void)
{
	struct ilist a, b, c, d;

	llist_init(&a);
	llist_insert_next(&a, &b);
	llist_insert_next(&b, &c);
	llist_insert_next(&c, &d);

	if (check_linkage(null, &a, &b, &c, &d, null)) { return -1; }

	llist_swap(&a, &b);
	if (check_linkage(null, &b, &a, &c, &d, null)) { return -1; }

	llist_swap(&c, &d);
	if (check_linkage(null, &b, &a, &d, &c, null)) { return -1; }

	llist_swap(&b, &c);
	if (check_linkage(null, &c, &a, &d, &b, null)) { return -1; }

	llist_swap(&a, &d);
	if (check_linkage(null, &c, &d, &a, &b, null)) { return -1; }

	llist_swap(&b, &b);
	if (check_linkage(null, &c, &d, &a, &b, null)) { return -1; }

	return 0;
}

static int lswap_lists(void)
{
	struct ilist a0, a1, a2;
	struct ilist b0, b1, b2;

	llist_init(&a0);
	llist_insert_next(&a0, &a1);
	llist_insert_next(&a1, &a2);

	llist_init(&b0);
	llist_insert_next(&b0, &b1);
	llist_insert_next(&b1, &b2);

	if (check_linkage(null, &a0, &a1, &a2, null)) { return -1; }
	if (check_linkage(null, &b0, &b1, &b2, null)) { return -1; }

	llist_swap(&a1, &b1);

	if (check_linkage(null, &a0, &b1, &a2, null)) { return -1; }
	if (check_linkage(null, &b0, &a1, &b2, null)) { return -1; }

	return 0;
}

static int lremove(void)
{
	struct ilist a, b, c;

	llist_init(&a);
	llist_insert_next(&a, &b);
	llist_remove(&b);
	if (check_linkage(null, &a, null)) { return -1; }

	llist_init(&a);
	llist_insert_next(&a, &b);
	llist_remove(&a);
	if (check_linkage(null, &b, null)) { return -1; }

	llist_init(&a);
	llist_insert_next(&a, &b);
	llist_insert_next(&b, &c);
	llist_remove(&c);
	if (check_linkage(null, &a, &b, null)) { return -1; }

	llist_init(&a);
	llist_insert_next(&a, &b);
	llist_insert_next(&b, &c);
	llist_remove(&a);
	if (check_linkage(null, &b, &c, null)) { return -1; }

	llist_init(&a);
	llist_insert_next(&a, &b);
	llist_insert_next(&b, &c);
	llist_remove(&b);
	if (check_linkage(null, &a, &c, null)) { return -1; }

	return 0;
}


static int cinit(void)
{
	struct ilist node;
	clist_init(&node);
	return check_linkage(&node, &node);
}

static int cinsert_next(void)
{
	struct ilist a, b;
	clist_init(&a);
	clist_insert_next(&a, &b);
	return check_linkage(&a, &b, &a);
}

static int csingleton(void)
{
	struct ilist a, b;
	clist_init(&a);
	if (!clist_singleton(&a)) { return -1; }
	clist_insert_next(&a, &b);
	if (clist_singleton(&a)) { return -2; }
	if (clist_singleton(&b)) { return -3; }
	return 0;
}

static int cinsert_prev(void)
{
	struct ilist a, b;
	clist_init(&a);
	clist_insert_prev(&a, &b);
	return check_linkage(&a, &b, &a);
}

static int ccreate_list(void)
{
	struct ilist a, b, c, d, e;
	clist_init(&b);
	clist_insert_next(&b, &d);
	clist_insert_next(&b, &c);
	clist_insert_prev(&b, &a);
	clist_insert_prev(&a, &e);
	return check_linkage(&a, &b, &c, &d, &e, &a);
}

static int clength(void)
{
	struct ilist a, b, c;

	clist_init(&a);
	if (clist_length(&a) != 1) { return -1; }
	clist_insert_next(&a, &b);
	if (clist_length(&a) != 2) { return -1; }
	if (clist_length(&a) != clist_length(&b)) { return -1; }
	clist_insert_next(&b, &c);
	if (clist_length(&a) != 3) { return -1; }
	if (clist_length(&a) != clist_length(&b)) { return -1; }
	if (clist_length(&a) != clist_length(&c)) { return -1; }
	return 0;
}

static int cswap_order(void)
{
	struct ilist a, b, c, d;

	clist_init(&a);
	clist_insert_next(&a, &b);

	clist_swap(&a, &b);
	if (check_linkage(&a, &b, &a)) { return -1; }

	clist_insert_next(&a, &c);
	llist_swap(&b, &c);
	if (check_linkage(&a, &b, &c, &a)) { return -1; }

	clist_insert_prev(&b, &d);
	llist_swap(&d, &c);
	if (check_linkage(&a, &c, &b, &d, &a)) { return -1; }

	return 0;
}

static int cswap_lists(void)
{
	struct ilist a0, a1, a2;
	struct ilist b0, b1, b2;

	clist_init(&a0);
	clist_insert_next(&a0, &a1);
	clist_insert_next(&a1, &a2);

	clist_init(&b0);
	clist_insert_next(&b0, &b1);
	clist_insert_next(&b1, &b2);

	if (check_linkage(&a0, &a1, &a2, &a0)) { return -1; }
	if (check_linkage(&b0, &b1, &b2, &b0)) { return -1; }

	clist_swap(&a1, &b1);

	if (check_linkage(&a0, &b1, &a2, &a0)) { return -1; }
	if (check_linkage(&b0, &a1, &b2, &b0)) { return -1; }

	return 0;
}

static int cremove(void)
{
	struct ilist a, b, c;

	clist_init(&a);
	clist_insert_next(&a, &b);
	clist_remove(&b);
	if (check_linkage(&a, &a)) { return -1; }

	clist_init(&a);
	clist_insert_next(&a, &b);
	clist_insert_next(&b, &c);
	clist_remove(&c);
	if (check_linkage(&b, &a, &b)) { return -1; }

	return 0;
}

struct test const tests[] = {
	{ test_check_fail, "test function" },

	{ linit, "initialize linear lists" },
	{ linsert_next, "insert a linear node after another" },
	{ lsingleton, "a single element linear list is a singleton" },
	{ linsert_prev, "insert a linear node before another" },
	{ lcreate_list, "create a linear lists" },
	{ llength, "length of linear list" },
	{ lswap_order, "swap order of elements within a linear list" },
	{ lswap_lists, "swap elements between linear lists" },
	{ lremove, "remove elements from linear lists" },

	{ cinit, "initialize circular lists" },
	{ cinsert_next, "insert a circular node after another" },
	{ csingleton, "a single element circular list is a singleton" },
	{ cinsert_prev, "insert a circular node before another" },
	{ ccreate_list, "create a circular lists" },
	{ clength, "length of circular list" },
	{ cswap_order, "swap order of elements within a circular list" },
	{ cswap_lists, "swap elements between circular lists" },
	{ cremove, "remove elements from circular lists" },

	{ NULL, NULL }
};
