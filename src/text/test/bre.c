
/* Functional tests of re.c */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ok/ok.h>
#include "../include/re.h"

static void check(char const *re, int matches, char const *text)
{
	if (rematch(re, text)) {
		if (!matches) {
			printf("Match: /%s/ ~ \"%s\"\n", re, text);
			ok = -1;
		}
	} else if (matches) {
		printf("Mismatch: /%s/ !~ \"%s\"\n", re, text);
		ok = -1;
	}
}

static void checkn(char const *re, int target, int n, va_list ap)
{
	int i;
	char const *p;

	for (i = 0; i < n; i++) {
		p = va_arg(ap, char const *);
		check(re, target, p);
	}
}

static void check_match(char const *re, char const *text)
{
	check(re, 1, text);
}

static void check_matchn(char const *re, int n, ...)
{
	va_list ap;

	va_start(ap, n);
	checkn(re, 1, n, ap);
	va_end(ap);
}

static void check_mismatch(char const *re, char const *text)
{
	check(re, 0, text);
}

static void check_mismatchn(char const *re, int n, ...)
{
	va_list ap;

	va_start(ap, n);
	checkn(re, 0, n, ap);
	va_end(ap);
}

static void check_invalid(char const *re)
{
	int n = recount(re);
	if (n >= 0) {
		printf("Failed to invalidate: /%s/ (%d groups)\n", re, n);
		ok = -1;
	}
}

static void check_count(char const *re, int exp)
{
	int n = recount(re);
	if (n < 0) {
		printf("Failed to validate: /%s/\n", re);
		ok = -1;
	} else if (n != exp) {
		printf("Found %d groups, expected %d: /%s/\n", n, exp, re);
		ok = -1;
	}
}

static void check_captures(const char *re, const char *text, ...)
{
	va_list ap;
	char const *s;
	int i, ncap, off, len, slen, pre;
	struct recap *cap;

	ncap = recount(re); 
	if (ncap < 0) {
		printf("Invalid regular expression /%s/\n", re);
		ok = -1;
		return;
	}
	cap = malloc(ncap * sizeof *cap);
	if (recap(re, text, cap) < 0) {
		printf("Failed to match /%s/\n", re);
		goto end;
	}
	va_start(ap, text);
	for (i = 0, pre = 0; i < ncap; i++) {
		off = va_arg(ap, int);
		len = va_arg(ap, int);
		if (cap[i].offset != off || cap[i].length != (size_t)len) {
			if (cap[i].offset < 0 ||
			    (size_t)cap[i].offset > strlen(text)) {
				s = NULL;
				slen = 0;
			} else {
				s = text + cap[i].offset;
				slen = strlen(s);
				if ((size_t)slen > cap[i].length) {
					slen = cap[i].length;
				}
			}
			if (!pre) {
				pre = 1;
				printf("/%s/ ~ \"%s\"\n", re, text);
			}
			printf("\tcapture %d: expected \"%.*s\" (%d, %d),"
			       " got \"%.*s\" (%d, %zd)\n", i, len,
			       text + off, off, len, slen, s, cap[i].offset,
			       cap[i].length);
			ok = -1;
		}
	}
	va_end(ap);

end:	free(cap);
	cap = NULL;
}

static int simple(void)
{
	/* Empty regular expression always matches */
	check_match("", "");
	check_match("", "abcdef");

	/* Exact matches */
	check_match("a", "a");
	check_match("ab", "ab");
	check_match("abc", "abc");
	check_mismatch("b", "a");
	check_mismatch("aa", "ab");
	check_mismatch("abb", "abc");

	/* Match somewhere in string */ 
	check_match("a", "xyzaxyz");
	check_match("ab", "inabad");
	check_match("abc", "abcabcabc");
	check_mismatch("abc", "abaabbabdabe");

	return ok;
}

static int caret(void)
{
	check_match("^", "");
	check_match("^", "abc");

	check_match("^abc", "abc");
	check_match("abc", "foo abc bar");
	check_mismatch("^abc", "foo abc bar");

	return ok;
}

static int dollar(void)
{
	check_match("$", "");
	check_match("^", "abc");

	check_match("abc$", "abc");
	check_match("abc", "foo abc bar");
	check_mismatch("abc$", "foo abc bar");

	return ok;
}

static int whole(void)
{
	check_match("^$", "");
	check_mismatch("^$", "x");

	check_match("^abc$", "abc");
	check_mismatch("^abc$", "xabc");
	check_mismatch("^abc$", "abcx");

	check_match("abc$", "abc");
	check_match("abc", "foo abc bar");
	check_mismatch("abc$", "foo abc bar");

	return ok;
}

static int quantify(void)
{
	check_matchn("a{0}", 2, "", "x"); 

	check_matchn("a{0,1}", 6, "", "a", "x", "xa", "ax", "xax", "aa");
	check_matchn("a?",     6, "", "a", "x", "xa", "ax", "xax", "aa");

	check_matchn("a{0,}", 7, "", "xa", "ax", "xax", "a", "aa", "aaa");
	check_matchn("a*",    7, "", "xa", "ax", "xax", "a", "aa", "aaa");

	check_matchn("a{1,}", 6, "a", "xa", "ax", "xax", "aa", "aaa");
	check_matchn("a+",    6, "a", "xa", "ax", "xax", "aa", "aaa");
	check_mismatchn("a+", 2, "", "x");

	check_matchn("a{2,3}", 5, "aa", "aaa", "aaaa", "baab", "babaaab");
	check_mismatchn("a{2,3}", 2, "", "a");

	check_matchn("xa{2,3}ax", 2, "xaaax", "xaaaax");
	check_mismatchn("xa{2,3}ax", 3, "xax", "xaax", "xaaaaax");

	return ok;
}

static int dot(void)
{
	check_matchn(".", 8, "a", "b", "c", "d", "e", "f", "g", "h");
	check_mismatch(".", "");

	check_match(".y.", "xyz");
	check_mismatchn(".y.", 2, "yz", "xy");
	check_match("y..y", "xyzzy");

	check_matchn(".?.?.?.?.?", 6, "", "a", "ab", "abc", "abcd", "abcde");

	return ok;
}

static int backslash(void)
{
	check_mismatchn("\\.", 8, "a", "b", "c", "d", "e", "f", "g", "h");
	check_match("\\.", ".");

	check_match("\\d", "abc1xyz");
	check_match("^\\s*foo\\s*bar\\s+baz\\s*$", " 	foobar baz	 ");
	check_match("\\(.*\\)", "a(xyz)b");
	check_match("a\\{2,6\\}", "a{2,6}");
	check_match("\\\\", "a\\b");

	/* Space */
	check_match("\\S\\s", "x ");

	/* Digits */
	check_match("\\D\\d", "x7");

	/* Words */
	check_matchn("\\w", 4, "x", "X", "7", "_");
	check_matchn("\\W", 4, ".", "?", " ", "\t");

	return ok;
}

static int classes(void)
{
	check_matchn("[abc]", 3, "axy", "xby", "xyc");
	check_matchn("[0-9]", 5, "0", "a1", "2b", "x8y", "9");

	return ok;
}

static int group(void)
{
	/* Empty group */
	check_match("()", "hello");
	check_match("()*", "hello");
	check_match("(((()*){0})*)*", "hello");
	check_match("(((()*)*)*)*", "hello");
	check_match("(((()*)*){0})*", "hello");
	check_match("(((()*){0})*)*", "hello");
	check_match("((((){0})*)*)*", "hello");
	check_match("((((x)*){0})*)*", "hello");
	check_match("((((x)*)*)*)*", "hello");
	check_match("((((x)*)*){0})*", "hello");
	check_match("((((x)*){0})*)*", "hello");
	check_match("((((x){0})*)*)*", "hello");
	check_match("((((x){0}y{0}((ll)z{0}))*)*)*", "llo");

	/* Simple groups */
	check_matchn("(.)", 8, "a", "b", "c", "d", "e", "f", "g", "h");
	check_matchn("(.*)..", 4, "ab", "abc", "abcd", "abcde");

	/* Quantified groups */
	check_matchn("(hello )?there", 2, "hello there", "there");
	check_matchn("(.)*..", 4, "ab", "abc", "abcd", "abcde");

	/* Complex cases */
	check_match("(12{1,3}(abc*))*", "hello");
	check_matchn("^.(ab)*.$", 3, "xy", "aabb", "babababa");
	check_matchn("1(0?1?)*", 3, "1", "11", "1001101001");
	check_mismatch("a*$", "aaab");
	check_mismatch("(a*$)", "aaab");
	check_match("a*$", "aaa");
	check_match("(a*$)", "aaa$b");

	return ok;
}

static int count(void)
{
	check_invalid("{");
	check_count("((((x){0}y{0}((ll)z{0}))*)*)*", 7);
	check_count("(?:(?:.*)(?:.*))*", 1);
	check_count("a", 1);
	check_count("(a)", 2);
	check_count("((.*)* a)", 3);
	check_count("((.*)* (a))", 4);
	check_count("((.*(.*))* (a))", 5);
	check_count("[()]*", 1);
	check_count("\\(\\)", 1);
	check_count("[()]*", 1);

	check_count("()", 2);
	check_count("()*", 2);
	check_count("(((()*){0})*)*", 5);
	check_count("(((()*)*)*)*", 5);
	check_count("(((()*)*){0})*", 5);
	check_count("(((()*){0})*)*", 5);
	check_count("((((){0})*)*)*", 5);
	check_count("((((x)*){0})*)*", 5);
	check_count("((((x)*)*)*)*", 5);
	check_count("((((x)*)*){0})*", 5);
	check_count("((((x)*){0})*)*", 5);
	check_count("((((x){0})*)*)*", 5);

	return ok;
}

static int alternate(void)
{
	/* Basic alternations */
	check_matchn("a|b", 2, "a", "b");
	check_matchn("hello|world|foo|bar", 4, "hello", "world", "foo", "bar");
	check_matchn("x(a|b)+y", 4, "xay", "xby", "xabbay", "xbababay");

	/* Empty expression in branch */
	check_matchn("|not this", 4, "", "anything", "should", "match");
	check_matchn("not this|", 4, "", "anything", "should", "match");

	/* Sanity checks */
	check_mismatch("a|b", "c");
	check_mismatchn("hello|world|foo|bar", 4, "helo", "word", "oof", "arb");

	return ok;
}

static int useful(void)
{
	char const *fp = "^[+-]?([0-9]+(\\.[0-9]*)?|\\.[0-9]+)([eE]-?[0-9]+)?$";
	char const *bin = "^[01]?(10|01)*|(10|01)*[01]$";

	/* Groups and anchors */
	check_match("^(a|b*)fo+(bar|baz)?...|something else$", "bbfoo123");
	check_match("^test|(not)?(this)?(a|c{2,3}|b)+$", "bcccbbcccccaccccaaa");
	check_match("^(a|(b*|cc)|ccc)+$", "bcccbbcccccaccccaaa");
	check_mismatch("^(a|(b*|cc)|ccc)+$", "bcccbbcccccdaccccaaa");
	check_match("^(xyz|abc)(a*|b*)aa$", "abcaa");
	check_match("^(cc|ccc)*$", "ccc");

	/* Flexible floating point */
	check_matchn(fp, 7, "0", "1.0", "1.", "1e6", "3.14", ".22500", "1e-4");
	check_matchn(fp, 3, "0.46234e-23", "+1", "-1");
	check_mismatchn(fp, 7, ".", "e", "E", ".e", "1.e", "+-1", "1.0-");
	check_mismatchn(fp, 5, "+.", "-e", "-E", "+.e", "-1.e-");

	/* A phony balanced binary code */
	check_matchn(bin, 4, "0", "1001", "01011010", "10101001");
	check_mismatchn(bin, 4, "11", "10001", "10110100", "1010110001");

	return ok;
}

static int capture(void)
{
	check_captures("needle", "there is a needle in here", 11, 6);
	check_captures("(\\d+)(.)", "abc 123x abc", 4, 4,/**/4, 3,/**/7, 1);
	check_captures("(.)*x", "x", 0, 1, -1, 0);
	check_captures("(.*)x", "x", 0, 1, 0, 0);

	/* Alternation */
	check_captures("(foo)|(bar)|(baz)", "a foo b",  2,3,  2,3, -1,0, -1,0);
	check_captures("(foo)|(bar)|(baz)", "a bar b",  2,3, -1,0,  2,3, -1,0);
	check_captures("(foo)|(bar)|(baz)", "a baz b",  2,3, -1,0, -1,0,  2,3);
	check_captures("(foo)|(bar)|(baz)", "a xxx b", -1,0, -1,0, -1,0, -1,0);

	/* Nested captures should capture only the first time. */
	check_captures("((.) (.) ?)*", "a b c d e", 0,8, 0,4, 0,1, 2,1);

	/* Branches that failed to match should not have any captures */
	check_captures("(.)x|(.)y", "ay", 0,2, -1,0, 0,1);

	return ok;
}

static int lazy(void)
{
	check_captures("\\d{2,}.", "123456789", 0,9);
	check_captures("\\d{2,}?.", "123456789", 0,3);
	check_captures("(\\d){2,}?.", "x123456789y", 1,3, 1,1);
	check_captures("((\\d){2,}?)*.", "123456789y", 0,9, 0,2, 0,1);
	check_captures("((\\d){2})*?[^0-9]", "12345678y", 0,9, 0,2, 0,1);
	return ok;
}

struct test const tests[] = {
	{ &simple,	"Simple regular expressions" },
	{ &caret,	"Start of string anchor" },
	{ &dollar,	"End of string anchor" },
	{ &whole,	"Match whole string" },
	{ &quantify,	"Simply quantified expressions" },
	{ &dot,		"Match any characer" },
	{ &backslash,	"Built-in character classes" },
	{ &classes,	"Custom character classes" },
	{ &group,	"Groups and quantified groups" },
	{ &count,	"Group count and validation" },
	{ &alternate,	"Alternation" },
	{ &useful,	"Useful expressions" },
	{ &capture,	"Text captures" },
	{ &lazy,	"Lazy quantifiers" },
	{ NULL, NULL }
};

