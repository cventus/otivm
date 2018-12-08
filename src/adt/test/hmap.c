#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>

#include "base/mem.h"
#include "base/wbuf.h"
#include "ok/ok.h"
#include "adt/hmap.h"

static void make_key(char *buf, size_t size, int i)
{
	snprintf(buf, size - 1, "key-%d", i);
	buf[size - 1] = '\0';
}

static int make(void)
{
	struct hmap hm;

	hmap_init(&hm, sizeof (int), alignof(int));
	hmap_term(&hm);

	hmap_init(&hm, sizeof (char *), alignof(char *));
	hmap_term(&hm);

	hmap_init(&hm, sizeof (double[42]), alignof(double [42]));
	hmap_term(&hm);

	return ok;
}

static int addval(void)
{
	struct hmap hm;
	int *p, *q;

	hmap_init(&hm, sizeof (int), alignof(int));
	p = hmap_gets(&hm, "key");
	if (p) fail_test("key already exists\n");
	p = hmap_news(&hm, "key");
	if (!p) fail_test("unable to create new value\n");
	*p = 42;
	q = hmap_gets(&hm, "key");
	if (!q) fail_test("couldn't find key\n");
	if (q != p) fail_test("couldn't find key\n");
	hmap_term(&hm);

	return ok;
}

static int addvals(void)
{
	char key[100];
	struct hmap hm;
	int *p, *q, i, max;

	hmap_init(&hm, sizeof (int), alignof(int));
	max = 10000;

	for (i = 0; i < max; i++) {
		make_key(key, sizeof key, i);
		p = hmap_gets(&hm, key);
		if (p) fail_test("key `%s` already exists\n", key);
		p = hmap_news(&hm, key);
		if (!p) fail_test("unable to create new value `%s`\n", key);
		*p = i;
		q = hmap_gets(&hm, key);
		if (!q) fail_test("couldn't find inserted key `%s`\n", key);
		if (q != p) fail_test("new'd and get'd key `%s` mismatch", key);
	}
	for (i = 0; i < max; i++) {
		make_key(key, sizeof key, i);
		p = hmap_gets(&hm, key);
		if (!p) fail_test("key `%s` no longer exists\n", key);
		if (*p != i) {
			fail_test(
				"value for `%s` corrupted! "
				"expected %zu, got %zu\n", key, i, *p);
		}
	}
	hmap_term(&hm);

	return ok;
}

static int remval(void)
{
	struct hmap hm;

	hmap_init(&hm, sizeof (int), alignof(int));
	if (!hmap_news(&hm, "key")) { ok = -1; printf("new failed\n"); }
	hmap_removes(&hm, "key");
	if (hmap_gets(&hm, "key")) { ok = -1; printf("couldn't remove\n"); }
	hmap_term(&hm);

	return ok;
}

static int remvals(void)
{
	char key[100];
	struct hmap hm;
	int *p, i, max;

	hmap_init(&hm, sizeof (char), alignof(char));
	max = 10000;

	for (i = 0; i < max; i++) {
		make_key(key, sizeof key, i);
		p = hmap_news(&hm, key);
		if (!p) fail_test("unable to create new value `%s`\n", key);
	}
	for (i = 0; i < max; i++) {
		make_key(key, sizeof key, i);
		p = hmap_gets(&hm, key);
		if (!p) fail_test("key `%s` no longer exists\n", key);
		hmap_removes(&hm, key);
		p = hmap_gets(&hm, key);
		if (p) fail_test("failed to remove key `%s`\n", key);
	}
	hmap_term(&hm);

	return ok;
}

static int simulation(void)
{
	struct hmap hm;
	size_t i, n_values;
	bool *is_present;
	char key[100];
	int id, *val, *got;

	n_values = 10000;
	is_present = malloc(sizeof is_present * n_values);
	for (i = 0; i < n_values; i++) {
		is_present[i] = false;
	}
	hmap_init(&hm, sizeof (int), alignof(int));

	for (i = 0; i < 100 * n_values; i++) {
		id = (i + rand()) % n_values;
		make_key(key, sizeof key, id);

		/* get */
		got = hmap_gets(&hm, key);
		if (is_present[id] && !got) {
			printf("value not found\n");
			ok = -1;
		} else if (!is_present[id] && got) {
			printf("nonexistent value found\n");
			ok = -1;
		} else if (got && *got != id) {
			printf("get: corrupt value\n");
			ok = -1;
		}
		switch (rand() & 0x3) {
		case 0: /* new */
			val = hmap_news(&hm, key);
			if (!val && !is_present[id]) {
				printf("new failed\n");
				ok = -1;
			} else if (val && (is_present[id] || val == got)) {
				printf("new created new existing mapping\n");
				ok = -1;
			} else if (val) {
				is_present[id] = true;
				*val = id;
			}
			break;
		case 1: /* put */
			val = hmap_puts(&hm, key);
			if (!val) {
				printf("put failed\n");
				ok = -1;
			} else if (is_present[id]) {
				if (val != got) {
					printf("put failed to get mapping\n");
					ok = -1;
				}
			} else {
				is_present[id] = true;
			}
			*val = id;
			break;
		default: /* remove */
			hmap_removes(&hm, key);
			if (got && hmap_gets(&hm, key) != NULL) {
				printf("remove failed\n");
				ok = -1;
			} else {
				is_present[id] = false;
			}
			break;
		}
	}
	hmap_term(&hm);
	free(is_present);

	return ok;
}

static int traverse(void)
{
	struct hmap hm;
	struct hmap_bucket *p;
	struct hmap_key hk;
	char key[100];
	size_t i, n_values, count;
	struct value {
		bool is_visited;
		size_t i;
	} *val;

	n_values = 10000;
	hmap_init(&hm, sizeof (struct value), alignof(struct value));

	/* populate */
	for (i = 0; i < n_values; i++) {
		make_key(key, sizeof key, i);

		val = hmap_news(&hm, key);
		if (!val) fail_test("unable to insert ``%s''\n", key);
		val->is_visited = false;
		val->i = i;
	}
	/* traverse */
	count = 0;
	for (p = hmap_first(&hm); p != NULL; p = hmap_next(&hm, p)) {
		val = hmap_value(&hm, p);
		make_key(key, sizeof key, val->i);
		hk = hmap_key(&hm, p);
		if (strlen(key) + 1 != hk.size) {
			printf("key length invalid during traversal\n");
			ok = -1;
		} else if (strcmp(hk.key, key) != 0) {
			printf("key contents invalid during traversal\n");
			ok = -1;
		} else if (val->is_visited) {
			printf("``%s'' already visited\n", key);
			ok = -1;
		} else {
			val->is_visited = true;
			count++;
		}
	}
	if (count < n_values) {
		fail_test("too few iterations: %zu < %zu\n", count, n_values);
	} else if (count > n_values) {
		fail_test("too many iterations: %zu > %zu\n", count, n_values);
	}
	/* check */
	for (i = 0; i < n_values; i++) {
		make_key(key, sizeof key, i);

		val = hmap_gets(&hm, key);
		if (!val) {
			printf("unable to retrieve ``%s''\n", key);
			ok = -1;
		} else if (!val->is_visited) {
			printf("not visited ``%s''\n", key);
			ok = -1;
		}
	}
	hmap_term(&hm);

	return ok;
}

static int load_factor(void)
{
	long i, max;
	size_t cap, newcap, j, n;
	struct wbuf results;
	double load, *p;
	struct hmap hm;

	hmap_init(&hm, 0, 0);
	max = 1000000;
	wbuf_init(&results);
	n = 0;

	for (i = 0; i < max; i++) {
		cap = hmap_capacity(&hm);
		load = hmap_load(&hm);
		if (hmap_newl(&hm, i) == NULL) {
			printf("unable to add key %ld\n", i);
			ok = -1;
			break;
		}
		newcap = hmap_capacity(&hm);
		if (newcap != cap && cap != 0) {
			wbuf_write(&results, &load, sizeof load);
			n++;
		}
	}
	hmap_term(&hm);
	load = 0.0;
	if (ok == 0 && n > 0) {
		p = results.begin;
		for (j = 0; j < n; j++) {
			load += p[j];
		}
		load /= n;
	}
	printf("average load factor before rehashing: %f\n", load);
	wbuf_term(&results);
	return ok;
}

struct test const tests[] = {
	{ make, "create and destroy various hash maps" },
	{ addval, "insert a value" },
	{ addvals, "insert several values" },
	{ remval, "insert and remove a value" },
	{ remvals, "add and remove many values" },
	{ simulation, "insertions, lookups and removals in random order" },
	{ traverse, "insert values and traverse over them" },
	{ load_factor, "average load factor before rehashing" },

	{ NULL, NULL }
};

