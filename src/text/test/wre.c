/* Internal tests for re.c */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ok/ok.h"

/* Include source to insert static functions into this translation unit. When
   building this test the module should be linked against on the library level
   only. */
#include "../re.c"

static char const *policy_name(enum quantpol policy)
{
	switch (policy) {
	case LAZY: return "LAZY";
	case GREEDY: return "GREEDY";
	default: return "UNKNOWN";
	}
}

static void assert_quantifier(
	char const *p,
	int n,
	size_t min,
	size_t max,
	int more, enum quantpol policy)
{
	int m, first;
	struct quant q;

	m = parse_quantifier(mkslice(p), &q);
	if (m != n) {
		if (n < 0) {
			printf("``%s'' was incorrectly accepted\n", p);
		} else {
			printf("``%.*s'': parsed too many characters %d\n",
			       n, p, m);
		}
		ok = -1;
		return;
	}
	if (n < 0) { return; }
	if (q.min != min ||
	    q.max != max ||
	    (q.more && !more) ||
	    q.policy != policy) {
		first = 1;
		printf("``%.*s'':", n, p);
		
		if (q.min != min) {
			printf(" min=%zd, expected %zd", q.min, min);
			first = 0;
		}
	    	if (q.max != max) {
			printf(first + "; max=%zd, expected %zd", q.max, max);
			first = 0;
		}
	    	if (q.more && !more) {
			printf(first + "; more=%d, expected %d", q.more, more);
			first = 0;
		}
	    	if (q.policy != policy) {
			printf(first + "; %s, expected %s",
			       policy_name(q.policy), policy_name(policy));
			first = 0;
		}
		printf("\n");
		ok = -1;
	}
}

static void assert_quantifier_error(char const *p)
{
	assert_quantifier(p, -1, 0, 0, 0, 0);
}

int test_parse_quantifiers(void)
{
	/* Missing quantifier */
	assert_quantifier("abc", 0, 1, 1, 0, GREEDY);

	/* Closed and open ranges */
	assert_quantifier("{42}xyz", 4, 42, 42, 0, GREEDY);
	assert_quantifier("{42,42}xyz", 7, 42, 42, 0, GREEDY);
	assert_quantifier("{42,}xyz", 5, 42, 0, 1, GREEDY);
	assert_quantifier("{42,43}xyz", 7, 42, 43, 0, GREEDY);
	assert_quantifier("{42}?xyz", 5, 42, 42, 0, LAZY);
	assert_quantifier("{42,}?xyz", 6, 42, 0, 1, LAZY);
	assert_quantifier("{42,43}?xyz", 8, 42, 43, 0, LAZY);
	assert_quantifier("{5,}a", 4, 5, 0, 1, GREEDY);
	assert_quantifier("{5,}?a", 5, 5, 0, 1, LAZY);

	/* Kleene star */
	assert_quantifier("*a", 1, 0, 0, 1, GREEDY);
	assert_quantifier("*?a", 2, 0, 0, 1, LAZY);
	assert_quantifier("{0,}a", 4, 0, 0, 1, GREEDY);
	assert_quantifier("{0,}?a", 5, 0, 0, 1, LAZY);

	/* Plus */
	assert_quantifier("+a", 1, 1, 0, 1, GREEDY);
	assert_quantifier("+?a", 2, 1, 0, 1, LAZY);

	/* Optional */
	assert_quantifier("?a", 1, 0, 1, 0, GREEDY);
	assert_quantifier("??a", 2, 0, 1, 0, LAZY);

	/* Errors */
	assert_quantifier_error("{abc");
	assert_quantifier_error("{1,2,3}xyz");
	assert_quantifier_error("{42,41}xyz");
	assert_quantifier_error("{-1,2}xyz");
	assert_quantifier_error("{a,b}xyz");

	return ok;
}

int test_character_classes(void)
{
	int res;
	matchfn *match;

	res = parse_class(mkslice("[abc]"), &match);

	if (res != 5 || match == match_complement) {
		ok = -1;
	}

	res = match_class(mkslice("[abc]"), mkslice("c"));
	if (!res) {
		ok = -1;
	}

	return ok;
}

static void assert_end_anchor(char const *str)
{
	if (!has_end_anchor(str)) { ok = -1; }
}

static void assert_no_end_anchor(char const *str)
{
	if (has_end_anchor(str)) { ok = -1; }
}

int test_end_of_string(void)
{
	assert_end_anchor("$");
	assert_end_anchor("abc$");
	assert_end_anchor("x\\\\$");

	assert_no_end_anchor("\\$");
	assert_no_end_anchor("abc\\\\\\$");
	assert_no_end_anchor("\\\\\\\\\\$");

	return ok;
}
