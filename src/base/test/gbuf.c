
#include <stdio.h>
#include <string.h>
#include <stdalign.h>
#include <stddef.h>

#include <ok/ok.h>
#include <base/gbuf.h>

#define assert_streq(a, b) assert_streq_(a, b, strlen(b))
static void assert_streq_(char const *a, char const *b, size_t n)
{
	if (strncmp(a, b, n) != 0) {
		printf("got: %.*s\nexpected: %.*s\n", (int)n, a, (int)n, b);
		ok = -1;
	}
}

static void assert_sizes(struct gbuf const *buf)
{
	size_t capacity, used, available;

	capacity = gbuf_capacity(buf);
	used = gbuf_size(buf);
	available = gbuf_available(buf);

	if (capacity != used + available) {
		printf("Inconsistent state: capacity(%zd) != "
		       "used(%zd) + available(%zd) = %zd\n", capacity, used,
		       available, used + available);
		ok = -1;
	}
}

static void assert_quantity(
	struct gbuf const *buf,
	size_t (*fn)(struct gbuf const *),
	const char *name,
	size_t expected)
{
	size_t n;
	n = fn(buf);
	if (n != expected) {
		printf("%s is %zd, expected %zd\n", name, n, expected);
		ok = -1;
	}
	assert_sizes(buf);
}

static void assert_quantity_ge(
	struct gbuf const *buf,
	size_t (*fn)(struct gbuf const *),
	const char *name,
	size_t expected)
{
	size_t n;
	n = fn(buf);
	if (n < expected) {
		printf("%s is %zd, expected at least %zd\n", name, n, expected);
		ok = -1;
	}
	assert_sizes(buf);
}

static void assert_used(struct gbuf const *buf, size_t expected)
{
	assert_quantity(buf, gbuf_size, "Used", expected);
}

static void assert_capacity(struct gbuf const *buf, size_t expected)
{
	assert_quantity(buf, gbuf_capacity, "Capacity", expected);
}

static void assert_capacity_ge(struct gbuf const *buf, size_t min)
{
	assert_quantity_ge(buf, gbuf_capacity, "Capacity", min);
}

static void assert_available(struct gbuf const *buf, size_t expected)
{
	assert_quantity(buf, gbuf_available, "Available", expected);
}

static void try_reserve(struct gbuf *buf, size_t alloc_size)
{
	if (gbuf_reserve(buf, alloc_size) != 0) {
		bail_out("Out of memory!");
	}
}

static void *try_alloc(struct gbuf *buf, size_t size)
{
	void *result = gbuf_alloc(buf, size);
	if (!result) {
		bail_out("Out of memory!");
	}
	return result;
}

static void *try_write(struct gbuf *buf, void const *data, size_t size)
{
	void *result = gbuf_write(buf, data, size);
	if (!result) {
		bail_out("Out of memory!");
	}
	return result;
}

static int empty(void)
{
	assert_used(&empty_gbuf, 0);
	assert_available(&empty_gbuf, 0);
	assert_capacity(&empty_gbuf, 0);
	return ok;
}

static int init(void)
{
	struct gbuf buf;

	init_gbuf(&buf);
	assert_used(&buf, 0);
	assert_available(&buf, 0);
	assert_capacity(&buf, 0);
	if (gbuf_trim(&buf)) { fail_test("failed to trim"); }
	if (gbuf_move_to(&buf, 0)) { fail_test("failed to move"); }
	if (gbuf_align(&buf, 8)) { fail_test("failed to align"); }
	if (gbuf_retract(&buf, 0)) { fail_test("failed to retract"); }
	gbuf_append(&buf);
	gbuf_prepend(&buf);
	term_gbuf(&buf);

	return ok;
}

static int reserve(void)
{
	struct gbuf buf;

	init_gbuf(&buf);
	
	assert_capacity(&buf, 0);
	assert_sizes(&buf);
	try_reserve(&buf, 100);
	assert_capacity(&buf, 100);
	assert_sizes(&buf);
	try_reserve(&buf, 10);
	assert_capacity(&buf, 100);
	assert_sizes(&buf);
	try_reserve(&buf, 1000);
	assert_capacity_ge(&buf, 1000);
	assert_sizes(&buf);

	term_gbuf(&buf);

	assert_capacity(&buf, 0);
	assert_sizes(&buf);

	return ok;
}

static int alloc(void)
{
	struct gbuf buf;
	size_t i;

	init_gbuf(&buf);
	
	for (i = 1; i <= 100; i++) {
		try_alloc(&buf, 100);
		assert_used(&buf, i * 100);
	}

	term_gbuf(&buf);

	assert_used(&buf, 0);
	assert_capacity(&buf, 0);

	return ok;
}

static int write(void)
{
	struct gbuf buf;
	size_t i, len;

	char const *data = "hello, world";

	init_gbuf(&buf);
	
	for (i = 0, len = strlen(data); i < len; i++) {
		try_write(&buf, data + i, 1);
	}

	assert_used(&buf, len);

	if (gbuf_capacity(&buf) < len || !buf.lbegin) {
		printf("Not enough space in buffer!\n");
		ok = -1;
	} else if (strncmp(buf.lbegin, data, len) != 0) {
		printf("Buffer content doesn't match what was written!\n");
		ok = -1;
	}

	term_gbuf(&buf);
	assert_used(&buf, 0);
	assert_capacity(&buf, 0);

	return ok;
}

static int copy(void)
{
	struct gbuf buf;
	size_t i;
	int const data[6] = { -1, 0, 1, 2, 3, 4 };
	int target[6];

	init_gbuf(&buf);
	
	for (i = 0; i < 6; i++) {
		try_write(&buf, data + i, sizeof(data[i]));
	}

	assert_used(&buf, sizeof data);

	if (gbuf_capacity(&buf) < sizeof data || !buf.lbegin) {
		printf("Not enough space in buffer!\n");
		ok = -1;
	} else {
		if (gbuf_copy(target, &buf) != target) {
			printf("gbuf_copy should return target pointer\n");
			ok = -1;
		}
		if (memcmp(target, data, sizeof data) != 0) {
			printf("Copy content doesn't match source data!\n");
			ok = -1;
		}
	}

	term_gbuf(&buf);
	assert_used(&buf, 0);
	assert_capacity(&buf, 0);

	return ok;
}

static int align(void)
{
	/* This test might end with a unaligned memory access error on some
	   systems, if the implementaiton is faulty. Otherwise at least
	   valgrind should notice if something's wrong. */
	struct gbuf buf;
	char c = 1;
	short int s = 2;
	int i = 3;
	long int l = 4;
	double d = 5;

	init_gbuf(&buf);

	if (gbuf_align(&buf, alignof(c))) { fail_test("out of memory\n"); }
	gbuf_write(&buf, &c, sizeof c);

	if (gbuf_align(&buf, alignof(s))) { fail_test("out of memory\n"); }
	gbuf_write(&buf, &s, sizeof s);

	if (gbuf_align(&buf, alignof(i))) { fail_test("out of memory\n"); }
	gbuf_write(&buf, &i, sizeof i);

	if (gbuf_align(&buf, alignof(l))) { fail_test("out of memory\n"); }
	gbuf_write(&buf, &l, sizeof l);

	if (gbuf_align(&buf, alignof(d))) { fail_test("out of memory\n"); }
	gbuf_write(&buf, &d, sizeof d);

	term_gbuf(&buf);

	return ok;
}

static int edit(void)
{
	struct gbuf buf;

	init_gbuf(&buf);
	gbuf_reserve(&buf, 1);

	try_write(&buf, "llo", 3);
	assert_streq(buf.lbegin, "llo");
	try_write(&buf, "rld", 3);
	assert_streq(buf.lbegin, "llorld");
	gbuf_move_to(&buf, 3);
	assert_streq(buf.lbegin, "llo");
	assert_streq(buf.rbegin, "rld");
	try_write(&buf, ", wo", 4);
	assert_streq(buf.lbegin, "llo, wo");
	assert_streq(buf.rbegin, "rld");
	gbuf_prepend(&buf);
	try_write(&buf, "he", 2);
	assert_streq(buf.lbegin, "he");
	assert_streq(buf.rbegin, "llo, world");
	gbuf_append(&buf);
	try_write(&buf, "", 1);
	assert_sizes(&buf);
	gbuf_move_to(&buf, 5);
	gbuf_trim(&buf);

	assert_sizes(&buf);
	if (memcmp(buf.lbegin, "hello, world", 13)) {
		fail_test("failed to produce target string");  
	}
	try_write(&buf, " ", 1);
	try_write(&buf, "t", 1);
	try_write(&buf, "h", 1);
	try_write(&buf, "e", 1);
	try_write(&buf, "r", 1);
	try_write(&buf, "e", 1);
	assert_streq(buf.lbegin, "hello there");
	assert_streq(buf.rbegin, ", world");

	gbuf_retract(&buf, 4);
	gbuf_write(&buf, "o you again and again and again", 31);
	gbuf_write(&buf, "xxxxxxxxxx", 10);
	gbuf_write(&buf, "xxxxxxxxxx", 10);
	gbuf_write(&buf, "xxxxxxxxxx", 10);
	gbuf_write(&buf, "xxxxxxxxxx", 10);
	gbuf_write(&buf, "xxxxxxxxxx", 10);
	assert_streq(buf.lbegin, "hello to you again and again and again");
	assert_streq(buf.rbegin, ", world");

	gbuf_move_by(&buf, 1);
	gbuf_retract(&buf, 27 + 50);

	gbuf_trim(&buf);
	assert_streq(buf.lbegin, "hello to you world");

	term_gbuf(&buf);

	return ok;
}

struct test const tests[] = {
	{ empty, 	"validate the empty gap buffer" },
	{ init, 	"initiate" },
	{ reserve, 	"reserve memory" },
	{ alloc, 	"allocate memory" },
	{ write, 	"write data" },
	{ copy, 	"copy data" },
	{ align, 	"align write pointer" },
	{ edit, 	"write, retract, move, and trim" },
	{ NULL, NULL }
};

