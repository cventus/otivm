
#include <stdio.h>
#include <string.h>
#include <stdalign.h>
#include <ok/ok.h>

#include <rescache/rescache.h>

struct text
{
	enum { plain, rich } type;
	char const *filename;
};

struct tally
{
	size_t plain_count, rich_count;
};

static int ends_with(char const *str, char const *suffix)
{
	size_t len = strlen(str);
	char const *p = strstr(str, suffix);
	return p && p - str + strlen(suffix) == len;
}

static int load_txt(void const *key, size_t keysz, void *data, void *link)
{
	struct tally *tally = link;
	struct text *text = data;
	if (!ends_with(key, ".txt")) return -1;
	if (strlen(key) + 1 != keysz) {
		printf("%s: invalid key size\n", __func__);
		ok = -1;
	}
	text->filename = key;
	text->type = plain;
	tally->plain_count++;
	return 0;
}

static int load_rtf(void const *key, size_t keysz, void *data, void *link)
{
	struct tally *tally = link;
	struct text *text = data;
	if (!ends_with(key, ".rtf")) return -1;
	if (strlen(key) + 1 != keysz) {
		printf("%s: invalid key size\n", __func__);
		ok = -1;
	}
	text->filename = key;
	text->type = rich;
	tally->rich_count++;
	return 0;
}

static int (*const make_documents[])(void const *, size_t, void *, void *) = {
	load_txt,
	load_rtf
};

static void free_doc(void const *key, size_t key_size, void *data, void *link)
{
	struct tally *tally = link;
	struct text *text = data;

	(void)key_size;

	if (strcmp(text->filename, key) != 0) {
		printf("filename-key mismatch\n");
		ok = -1;
	}

	switch (text->type) {
	case rich: tally->rich_count--; break;
	case plain: tally->plain_count--; break;
	default:
		printf("Unknown type %d\n", (int)text->type);
		ok = -1;
	}
}

static int empty(void)
{
	struct rescache *r;

	r = make_rescachen(
		sizeof (struct text),
		alignof (struct text),
		alignof (char),
		make_documents,
		sizeof make_documents / sizeof make_documents[0],
		free_doc,
		NULL);

	if (!r) fail_test("out of memory\n");
	if (rescache_size(r) != 0) { ok = -1; }
	if (rescache_unused(r) != 0) { ok = -1; }
	if (free_rescache(r)) { ok = -1; }

	return ok;
}

static int create(void)
{
	struct rescache *r;
	struct tally counts = { 0, 0 };
	struct text *text;

	r = make_rescachen(
		sizeof (struct text),
		alignof (struct text),
		alignof (char),
		make_documents,
		sizeof make_documents / sizeof make_documents[0],
		free_doc,
		&counts);

	/* Create resource */
	text = rescache_loads(r, "file.txt");
	if (!text) fail_test("unable to load txt file\n");
	if (strcmp(text->filename, "file.txt") != 0) {
		printf("text file not initialized properly\n");
		ok = -1;
	}
	if (counts.plain_count != 1) {
		fail_test("context not updated properly\n");
	}
	if (rescache_size(r) != 1) {
		printf("invalid cache size\n");
		ok = -1;
	}
	if (rescache_unused(r) != 0) {
		printf("invalid unused count\n");
		ok = -1;
	}

	/* Free resource */
	if (free_rescache(r) == 0) {
		fail_test("free succeeded despite one resource in use\n");
	}
	rescache_release(r, text);
	if (counts.plain_count != 1) {
		printf("resource freed too soon\n");
		ok = -1;
	}
	if (rescache_size(r) != 1) {
		printf("invalid cache size\n");
		ok = -1;
	}
	if (rescache_unused(r) != 1) {
		printf("invalid unused count\n");
		ok = -1;
	}

	/* Reload resource */
	if (rescache_loads(r, "file.txt") != text) {
		fail_test("resource not reused\n");
	}
	rescache_release(r, text);
	if (free_rescache(r)) {
		printf("unable to free cache with unreferenced resources\n");
		ok = -1;
	}

	if (counts.plain_count != 0) {
		printf("resource not freed\n");
	}

	return ok;
}

struct test const tests[] = {
	{ empty, "create and free empty cache" },
	{ create, "create and recreate a single resource" },

	{ NULL, NULL }
};

