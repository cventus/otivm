#include <stdio.h>
#include <math.h>

#include "ok/ok.h"
#include "spline/shape.h"
#include "test.h"

#define check(cond) do { if (!(cond)) fail_test(#cond " failed\n"); } while (0)

static int test_already_simple(void)
{
	struct spline_shape s = {
		.n = 1,
		.outlines = (struct spline_outline[]) {
			{
				.n = 4,
				.segments = (struct spline_segment[]) {
					{ { 0.f, 1.f }, { 1.f, 1.f }, 1.f },
					{ { 1.f, 0.f }, { 1.f,-1.f }, 1.f },
					{ { 0.f,-1.f }, {-1.f,-1.f }, 1.f },
					{ {-1.f, 0.f }, {-1.f, 1.f }, 1.f },
				}
			}
		}
	};

	struct spline_shape *simplified;

	simplified = spline_simplify_shape(&s);
	check(simplified->n == 1);
	check(simplified->outlines->n == 4);
	spline_free_shape(simplified);

	return ok;
}

static int test_crescent(void)
{
	struct spline_shape s = {
		.n = 1,
		.outlines = (struct spline_outline[]) {
			{
				.n = 2,
				.segments = (struct spline_segment[]) {
					{ {-1.f, 0.f }, {-1.f, 1.f }, 1.f },
					{ { 0.f, 1.f }, {-0.9f, 0.9f }, 1.f }
				}
			}
		}
	};

	struct spline_shape *simplified;

	simplified = spline_simplify_shape(&s);
	check(simplified->n == 1);
	check(simplified->outlines->n == 10);
	spline_free_shape(simplified);

	return ok;
}


struct test const tests[] = {
	{ test_already_simple, "no simplification needed" },
	{ test_crescent, "simplify simple concave shape" },

	{ NULL, NULL }
};
