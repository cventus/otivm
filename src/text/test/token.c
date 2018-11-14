#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ok/ok.h"
#include "text/token.h"

static int assert_tokens(const char *line, int n, ...)
{
	const char *p, *q;
	char *copy;
	int i, result, t;
	va_list ap;

	copy = malloc(strlen(line) + 1);
	strcpy(copy, line);
	t = tokenize(copy, copy, 0);
	if (t != n) {
		printf("Expected %d tokens, got %d!", n, t);
		result = 0;
	} else {
		result = 1;
		q = copy;

		va_start(ap, n);
		for (i = 0; i < n; i++) {
			p = va_arg(ap, const char *);
			if (strcmp(p, q) != 0) {
				result = 0;
				printf("%d: expected ``%s'', got ``%s''!\n", i,
				       p, q);
				break;
			}
			q += strlen(p) + 1;
		}
		va_end(ap);
	}
	free(copy);

	if (!result) {
		printf("for ``%s''\n", line);
		ok = -1;
	}

	return result;
}

static int empty(void)
{
	assert_tokens("", 0);
	return ok;
}

static int space(void)
{
	assert_tokens(" ", 0);
	assert_tokens("\t", 0);
	assert_tokens("   ", 0);
	assert_tokens(" \t ", 0);
	return ok;
}

static int single(void)
{
	assert_tokens("a", 1, "a");
	assert_tokens("abc", 1, "abc");
	assert_tokens(" x", 1, "x");
	assert_tokens("x ", 1, "x");
	assert_tokens(" yz", 1, "yz");
	assert_tokens("yz ", 1, "yz");
	assert_tokens("   qwerty  ", 1, "qwerty");
	return ok;
}

static int several(void)
{
	assert_tokens("a b", 2, "a", "b");
	assert_tokens("abc def ghi", 3, "abc", "def", "ghi");
	assert_tokens(" ab cd ", 2, "ab", "cd");
	return ok;
}

static int quotes(void)
{
	assert_tokens("\"", 0);
	assert_tokens("   \"", 0);
	assert_tokens("  x\"", 1, "x");
	assert_tokens("\"\"", 1, "");
	assert_tokens("\"abc\"", 1, "abc");
	assert_tokens(" \"abc\"", 1, "abc");
	assert_tokens("\"abc\" ", 1, "abc");
	assert_tokens("  a\"b\"c  ", 1, "abc");
	assert_tokens(" a\"b  c\"d\" \" ", 1, "ab  cd ");
	assert_tokens(" \"a b c\" \"d e f\" ", 2, "a b c", "d e f");
	assert_tokens(" \" \" \"abc", 2, " ", "abc");
	return ok;
}

static int escape(void)
{
	assert_tokens("\\", 0);
	assert_tokens("  \\", 0);
	assert_tokens("  \\  ", 1, " ");
	assert_tokens("  \\ \\", 1, " ");
	assert_tokens("\\\"", 1, "\"");
	assert_tokens("\"\\\"\"", 1, "\"");
	assert_tokens(" \"\\\"\" ", 1, "\"");
	assert_tokens(" \"foo\\\"bar\"\\\" \\\"hello world\\\" ", 3,
	              "foo\"bar\"", "\"hello", "world\"");
	return ok;
}

struct test const tests[] = {
	{ empty,	"empty strings" },
	{ space,	"nothing but whitespace" },
	{ single,	"single token" },
	{ several,	"several tokens" },
	{ quotes,	"strings with quotes" },
	{ escape,	"strings with escaped quotes and spaces" },
	{ NULL,	NULL }
};

