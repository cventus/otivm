
#include <stdio.h>
#include <string.h>
#include <stdalign.h>

#include <ok/ok.h>
#include <base/wbuf.h>

static void assert_sizes(struct wbuf const *buf)
{
	size_t capacity, used, available;

	capacity = wbuf_capacity(buf);
	used = wbuf_used(buf);
	available = wbuf_available(buf);

	if (capacity != used + available) {
		printf("Inconsistent state: capacity(%zd) != "
		       "used(%zd) + available(%zd) = %zd\n", capacity, used,
		       available, used + available);
		ok = -1;
	}
}

static void assert_quantity(
	struct wbuf const *buf,
	size_t (*fn)(struct wbuf const *),
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
	struct wbuf const *buf,
	size_t (*fn)(struct wbuf const *),
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

static void assert_used(struct wbuf const *buf, size_t expected)
{
	assert_quantity(buf, wbuf_used, "Used", expected);
}

static void assert_capacity(struct wbuf const *buf, size_t expected)
{
	assert_quantity(buf, wbuf_capacity, "Capacity", expected);
}

static void assert_capacity_ge(struct wbuf const *buf, size_t min)
{
	assert_quantity_ge(buf, wbuf_capacity, "Capacity", min);
}

static void assert_available(struct wbuf const *buf, size_t expected)
{
	assert_quantity(buf, wbuf_available, "Available", expected);
}

static void try_reserve(struct wbuf *buf, size_t alloc_size)
{
	if (wbuf_reserve(buf, alloc_size) != 0) {
		bail_out("Out of memory!");
	}
}

static void *try_alloc(struct wbuf *buf, size_t size)
{
	void *result = wbuf_alloc(buf, size);
	if (!result) {
		bail_out("Out of memory!");
	}
	return result;
}

static void *try_write(struct wbuf *buf, void const *data, size_t size)
{
	void *result = wbuf_write(buf, data, size);
	if (!result) {
		bail_out("Out of memory!");
	}
	return result;
}

static int init(void)
{
	struct wbuf buf;

	wbuf_init(&buf);
	assert_used(&buf, 0);
	assert_available(&buf, 0);
	assert_capacity(&buf, 0);
	wbuf_free(&buf);

	return ok;
}

static int reserve(void)
{
	struct wbuf buf;

	wbuf_init(&buf);
	
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

	wbuf_free(&buf);

	assert_capacity(&buf, 0);
	assert_sizes(&buf);

	return ok;
}

static int alloc(void)
{
	struct wbuf buf;
	size_t i;

	wbuf_init(&buf);
	
	for (i = 1; i <= 100; i++) {
		try_alloc(&buf, 100);
		assert_used(&buf, i * 100);
	}

	wbuf_free(&buf);

	assert_used(&buf, 0);
	assert_capacity(&buf, 0);

	return ok;
}

static int write(void)
{
	struct wbuf buf;
	size_t i, len;

	char const *data = "hello, world";

	wbuf_init(&buf);
	
	for (i = 0, len = strlen(data); i < len; i++) {
		try_write(&buf, data + i, 1);
	}

	assert_used(&buf, len);

	if (wbuf_capacity(&buf) < len || !buf.begin) {
		printf("Not enough space in buffer!\n");
		ok = -1;
	} else if (strncmp(buf.begin, data, len) != 0) {
		printf("Buffer content doesn't match what was written!\n");
		ok = -1;
	}

	wbuf_free(&buf);
	assert_used(&buf, 0);
	assert_capacity(&buf, 0);

	return ok;
}

static int copy(void)
{
	struct wbuf buf;
	size_t i;
	int const data[6] = { -1, 0, 1, 2, 3, 4 };
	int target[6];

	wbuf_init(&buf);
	
	for (i = 0; i < 6; i++) {
		try_write(&buf, data + i, sizeof(data[i]));
	}

	assert_used(&buf, sizeof data);

	if (wbuf_capacity(&buf) < sizeof data || !buf.begin) {
		printf("Not enough space in buffer!\n");
		ok = -1;
	} else {
		if (wbuf_copy(target, &buf) != target) {
			printf("wbuf_copy should return target pointer\n");
			ok = -1;
		}
		if (memcmp(target, data, sizeof data) != 0) {
			printf("Copy content doesn't match source data!\n");
			ok = -1;
		}
	}

	wbuf_free(&buf);
	assert_used(&buf, 0);
	assert_capacity(&buf, 0);

	return ok;
}

static int align(void)
{
	/* This test might end with a unaligned memory access error on some
	   systems, if the implementaiton is faulty. Otherwise at least
	   valgrind should notice if something's wrong. */
	struct wbuf buf;
	char c = 1;
	short int s = 2;
	int i = 3;
	long int l = 4;
	double d = 5;

	wbuf_init(&buf);

	if (wbuf_align(&buf, alignof(c))) { fail_test("out of memory\n"); }
	wbuf_write(&buf, &c, sizeof c);

	if (wbuf_align(&buf, alignof(s))) { fail_test("out of memory\n"); }
	wbuf_write(&buf, &s, sizeof s);

	if (wbuf_align(&buf, alignof(i))) { fail_test("out of memory\n"); }
	wbuf_write(&buf, &i, sizeof i);

	if (wbuf_align(&buf, alignof(l))) { fail_test("out of memory\n"); }
	wbuf_write(&buf, &l, sizeof l);

	if (wbuf_align(&buf, alignof(d))) { fail_test("out of memory\n"); }
	wbuf_write(&buf, &d, sizeof d);

	wbuf_free(&buf);

	return ok;
}

struct test const tests[] = {
	{ init, 	"Initialization" },
	{ reserve, 	"Memory reservation" },
	{ alloc, 	"Memory allocation" },
	{ write, 	"Writing data" },
	{ copy, 	"Copying data" },
	{ align, 	"Align write pointer" },
	{ NULL, NULL }
};

