
#include "../include/list.h"
#include <ok/ok.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

struct entry
{
	int id;
	char name[];
};

static void check(int expr, char const *fmt, ...)
{
	va_list ap;

	if (!expr) {
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
		ok = -1;
	}
}

static void check_eq(int *list, int const *array, int n)
{
	int i, *p, len;

	len = list_length(list);
	if (len != n) {
		printf("list of length %d, expected %d\n", len, n);
		ok = -1;
		return;
	}

	for (i = 0, p = list; i < n; i++, p = list_next(p)) {
		check(*p == array[i], "%d == *p != array[%d] == %d\n",
		                       *p,               i,     array[i]);
	}
}

static struct entry *cons_entry(struct entry *list, int id, char const *name)
{
	struct entry *e;
	size_t len;

	len = strlen(name) + 1;
	e = list_prepend(list, sizeof *e + len);
	if (e) {
		e->id = id;
		strcpy(e->name, name);
	}
	return e;
}

static int example(void)
{
	static int const numbers[] = { 11, 42, 7, 13, 983 };

	int i, *head, *tail, n;

	n = sizeof numbers/sizeof 0[numbers];

	/* Append */
	head = tail = NULL;
	for (i = 0; i < n; i++) {
		tail = list_append(tail, sizeof *tail);
		if (head == NULL) {
			head = tail;
			check(list_prev(head) == NULL, "Previous of head "
			      "element should be NULL!\n");
			check(list_next(tail) == NULL, "Node following after "
			      "list tail should be NULL!\n");
		}
		*tail = numbers[i];
	}
	check_eq(head, numbers, n);
	list_free(head);

	/* Prepend */
	head = tail = NULL;
	for (i = n - 1; i >= 0; i--) {
		head = list_prepend(head, sizeof *head);
		if (tail == NULL) {
			tail = head;
			check(list_prev(head) == NULL, "Previous of head "
			      "element should be NULL!\n");
			check(list_next(tail) == NULL, "Node following after "
			      "list tail should be NULL!\n");
		}
		*head = numbers[i];
	}
	check_eq(head, numbers, n);
	list_free(tail);

	return ok;
}

static int people(void)
{
	char const *names[] = {
		"foo",
		"bar",
		"longer name",
		"",
		"xyz"
	};

	int i, n;
	struct entry *entries, *p;

	entries = NULL;
	n = (int)(sizeof names / sizeof 0[names]);

	for (i = n; i-- > 0; ) {
		entries = cons_entry(entries, i, names[i]);
	}

	/* Iterate forwards */
	for (p = list_head(entries), i = 0; i < n; i++, p = list_next(p)) {
		if (!p) { fail_test("unexpected end of list\n"); }
		check(strcmp(names[i], p->name) == 0, "%d: \"%s\" != \"%s\"\n",
		      i, names[i], p->name);
	}

	/* Iterate backwards */
	for (p = list_tail(entries), i = n; i-- > 0; p = list_prev(p)) {
		if (!p) { fail_test("unexpected end of list\n"); }
		check(strcmp(names[i], p->name) == 0, "%d: \"%s\" != \"%s\"\n",
		      i, names[i], p->name);
	}

	/* Remove in random order */
	p = list_remove(list_tail(entries));
	p = list_remove(list_head(p));
	while (p) {
		p = list_remove(p);
	}

	return ok;
}

struct test const tests[] = {
	{ example, "create and traverse lists" },
	{ people, "variable sized elements" },
	{ NULL, NULL }
};

