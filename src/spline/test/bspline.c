
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <ok/ok.h>
#include <base/mem.h>
#include <base/gbuf.h>

#include "../include/bspline.h"
#include "../include/types.h"

#define ASSERT(cond) do { \
		if (!(cond)) fail_test("failed: %s\n", #cond); \
	} while (0)

static float const points[] = {
	0.6f, 0.32f, 1.f,
	1.f, 0.5f, 0.5f,
	1.26f, 0.52f, 1.f,
	1.2f, 0.71f, 4.f,
	0.95f, 0.81f, 2.f,
	1.00f, 1.00f, 1.f,
	1.78f, 1.03f, 9.f,
	1.80f, 0.0f, 1.f,
};

static int make_zero_dimensional(void)
{
	struct bspline2 b;

	if (init_bspline2(&b, 0) == 0) {
		fail_test("creation of a 0-d b-spline caused no error\n");
	}
	return ok;
}

static int make_empty(void)
{
	struct bspline2 b;
	size_t i;

	/* typical numbers of dimensions */
	for (i = 1; i <= 4; i++) {
		if (init_bspline2(&b, i)) {
			fail_test("failed to create a %zd-d b-spline\n", i);
		}
		if (bspline2_size(&b) != 0) {
			fail_test("new b-spline not empty\n", i);
		}
		if (bspline2_period(&b) != 0.0) {
			fail_test("period of new b-spline not zero\n", i);
		}
		term_bspline2(&b);
	}
	return ok;
}

/*
static int eval_spline(void)
{
	struct bspline2 b;
	size_t i;
	float pt[2] = { 0.f, 0.f };
	double period;

	init_bspline2(&b, 3);

	bspline2_insert(&b, 0, length_of(points)/3, points);
	
	period = bspline2_period(&b);
	for (i = 0; i < 100; i++) {
		bspline2_reval(pt, &b, period * i/100.f);
		printf("%f\t%f\n", pt[0], pt[1]);
	}
	term_bspline2(&b);

	return ok;
}
*/

struct test const tests[] = {
	{ make_zero_dimensional, "prevent zero-dimensional points" }, 
	{ make_empty, "create and free splines" },

	{ NULL, NULL }
};

