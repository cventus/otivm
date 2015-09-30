
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ok/ok.h>
#include "../include/str.h"

void check_prefix(char *str, char const *example)
{
	if (strcmp(str, example) != 0) {
		printf("\"%s\" != \"%s\"\n", str, example);
		ok = -1;
	}
	free(str);
}

static int prefix(void)
{
	check_prefix(strdup_prefix("abc", 0), "");
	check_prefix(strdup_prefix("abc", 1), "a");
	check_prefix(strdup_prefix("abc", 2), "ab");
	check_prefix(strdup_prefix("abc", 3), "abc");
	check_prefix(strdup_prefix("abc", 4), "abc");
	return ok;
}

void check_fmt(char *str, char const *example, char *buf, int is_buf)
{
	if (strcmp(str, example) != 0) {
		printf("\"%s\" != \"%s\"\n", str, example);
		ok = -1;
	}
	if (is_buf) {
		if (str != buf) {
			printf("buffer wasn't returned!\n");
			ok = -1;
			free(str);
		 }
	} else {
		if (str == buf) {
			printf("buffer shouldn't have been returned!\n");
			ok = -1;
		} else {
			free(str);
		}
	}
}

static int format(void)
{
	char b[10];
	size_t z = sizeof b;

	check_fmt(strfmt(0, 0, "hello"), "hello", b, 0);
	check_fmt(strfmt(b, z, "hello"), "hello", b, 1);

	check_fmt(strfmt(b, z, "one two %d", 3), "one two 3", b, 1);
	check_fmt(strfmt(b, z, "one two %s", "three"), "one two three", b, 0);

	return ok;
}

struct test const tests[] = {
	{ prefix, "strdup_prefix" },
	{ format, "strfmt" },

	{ NULL, NULL }
};

