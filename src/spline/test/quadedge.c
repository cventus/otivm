#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ok/ok.h>
#include <assert.h>

#include <base/mem.h>
#include <base/wbuf.h>

#include "test.h"
#include "../geometry.h"
#include "../quadedge.h"

static inline void assert_eq(char const *as, char const *bs, eref a, eref b)
{
	if (a != b) {
		printf("not equal: %s = %ld != %ld = %s\n", as, (long)a, (long)b, bs);
		ok = -1;
	}
}

#define assert_eq(a, b) assert_eq(#a, #b, a, b)

/* specify orbit with a sequence of edge references, ending in the first */
static void assert_orbit(
	struct eset *set,
	eref next(struct eset *, eref),
	char const *nextname,
	char const *args,
	eref first,
	...)
{
	va_list ap;
	struct { eref ref; char const *name; int len; } prev, cur;
	static char const *delim = ", \t\n";

	prev.ref = first;
	prev.name = args;
	prev.len = strcspn(prev.name, delim);
	va_start(ap, first);
	do {
		cur.ref = va_arg(ap, eref);
		cur.name = prev.name + prev.len;
		cur.name += strspn(cur.name, delim);
		cur.len = strcspn(cur.name, delim);
		if (next(set, prev.ref) != cur.ref) {
			printf("%.*s is not followed by %.*s, ",
				prev.len, prev.name,
				cur.len, cur.name);
			printf("%s(%.*s) = %d, expected %d\n",
				nextname,
				prev.len, prev.name,
				(int)next(set, prev.ref), (int)cur.ref);
			ok = -1;
			break;
		}
		memcpy(&prev, &cur, sizeof cur);
	} while (cur.ref != first);
	va_end(ap);
}
#define assert_orbit(set, next, first, ...) \
	assert_orbit(set, next, #next,  #first ", " #__VA_ARGS__, first, __VA_ARGS__)

static eref ref(eref e, unsigned r) {
	return mkref(e*4, r);
}

static int test_make_edge(void)
{
	struct eset set;
	eref e;

	init_eset(&set);

	if (eset_alloc(&set, 1, &e)) { fail_test("allocation failure"); }
	assert_eq(lnext(&set, e), sym(e));
	assert_eq(rnext(&set, e), sym(e));
	assert_eq(onext(&set, e), e);
	assert_eq(oprev(&set, e), e);

	e = rot(e);
	assert_eq(lnext(&set, e), e);
	assert_eq(rnext(&set, e), e);
	assert_eq(onext(&set, e), sym(e));
	assert_eq(oprev(&set, e), sym(e));

	term_eset(&set);

	return ok;
}

static int traverse(void)
{
	enum { a, b, c, d, e, f, g, h };

	/* Based on Fig. 7. from Guibas & Stolfi (1985) */
	eref edges[] = {
		[a*4] = ref(g, 3), ref(g, 2), ref(a, 2), ref(a, 1),
		[b*4] = ref(h, 3), ref(a, 0), ref(a, 3), ref(c, 2),
		[c*4] = ref(d, 3), ref(b, 0), ref(b, 3), ref(b, 2),
		[d*4] = ref(c, 3), ref(h, 0), ref(c, 1), ref(c, 0),
		[e*4] = ref(d, 0), ref(e, 3), ref(e, 2), ref(d, 1),
		[f*4] = ref(e, 0), ref(g, 1), ref(h, 1), ref(e, 1),
		[g*4] = ref(f, 2), ref(f, 1), ref(f, 0), ref(h, 2),
		[h*4] = ref(f, 3), ref(g, 0), ref(b, 1), ref(d, 2)
	};
	struct eset set = {
		{ edges, edges + length_of(edges), NULL },
		{ NULL, NULL, NULL },
		0
	};

	assert_eq(onext(&set, ref(a, 2)), ref(a, 2));

	assert_eq(onext(&set, ref(a, 0)), ref(g, 3));
	assert_eq(onext(&set, ref(g, 3)), ref(h, 2));
	assert_eq(onext(&set, ref(h, 2)), ref(b, 1));
	assert_eq(onext(&set, ref(b, 1)), ref(a, 0));

	assert_eq(onext(&set, ref(d, 1)), ref(h, 0));
	assert_eq(onext(&set, ref(h, 0)), ref(f, 3));
	assert_eq(onext(&set, ref(f, 3)), ref(e, 1));
	assert_eq(onext(&set, ref(e, 1)), sym(ref(e, 1)));
	assert_eq(onext(&set, sym(ref(e, 1))), ref(d, 1));

	return ok;
}

static int test_connect(void)
{
	struct eset set;
	eref a, b, c, d, e, f, g, h;

	/* Create the following subdivision with canonical edges going counter
	   clock wise around the edges:

	    O-------O
	   /|   g   |\
	 d/ |       | \b
	 /  |       |  \
	O   |f     c|   O
	 \  |       |  /
	 e\ |       | /a
	   \|   h   |/
	    O-------O
	*/

	init_eset(&set);

	if (eset_alloc(&set, 8, &a, &b, &c, &d, &e, &f, &g, &h)) {
		fail_test("allocation failure");
	}

	/* create triangle on the right */
	eset_splice(&set, sym(a), b);
	eset_connect(&set, b, a, c);

	assert_orbit(&set, onext, a, sym(c), a);
	assert_orbit(&set, onext, b, sym(a), b);
	assert_orbit(&set, onext, c, sym(b), c);
	assert_orbit(&set, lnext, a, b, c, a);
	assert_orbit(&set, rnext, sym(a), sym(b), sym(c), sym(a));

	/* create triangle on the left */
	eset_splice(&set, sym(d), e);
	eset_connect(&set, e, d, f);

	assert_orbit(&set, onext, d, sym(f), d);
	assert_orbit(&set, onext, e, sym(d), e);
	assert_orbit(&set, onext, f, sym(e), f);
	assert_orbit(&set, lnext, d, e, f, d);
	assert_orbit(&set, rnext, sym(d), sym(e), sym(f), sym(d));

	/* connect the two triangles forming a quad in the middle */
	eset_connect(&set, sym(d), sym(b), g);
	eset_connect(&set, sym(a), sym(e), h);

	assert_orbit(&set, onext, g, d, sym(f), g);
	assert_orbit(&set, onext, c, sym(b), sym(g), c);
	assert_orbit(&set, onext, h, a, sym(c), h);
	assert_orbit(&set, onext, f, sym(e), sym(h), f);

	/* inner quad orbit */
	assert_orbit(&set, rnext, h, c, g, f, h);

	/* outer shape orbit */
	assert_orbit(&set, lnext, h, sym(e), sym(d), g, sym(b), sym(a), h);

	term_eset(&set);

	return ok;
}

struct test const tests[] = {
	{ test_make_edge, "properties of new subdivision" },
	{ traverse, "traverse edges connected to a vertex/around a face" },
	{ test_connect, "connect points" },

	{ NULL, NULL }
};
