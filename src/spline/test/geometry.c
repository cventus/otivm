#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <ok/ok.h>

#include "test.h"
#include "../geometry.h"

static int winding_order(void)
{
	typedef float xy[XY];

	xy a = { 0.0f, 0.0f };
	xy b = { 1.0f, 0.0f };
	xy c = { 0.0f, 1.0f };

	assert_true(line2d_det(a, b, c) > EPSILON);
	assert_false(line2d_det(a, c, b) > EPSILON);

	return ok;
}

static int test_in_circle(void)
{
	typedef float xy[XY];

	xy A = { 10.0f, 10.0f };
	xy B = { 11.0f, 10.0f };
	xy C = { 10.0f, 11.0f };

	xy D = { 10.1f, 10.1f };

	xy E = { 11.0f, 11.0f };
	xy F = { 12.0f, 12.0f };

	/* clearly inside */
	assert_true(point2d_in_circle(A, B, C, D));

	/* co-circular */
	assert_false(point2d_in_circle(A, B, C, E));

	/* clearly outside */
	assert_false(point2d_in_circle(A, B, C, F));

	return ok;
}

static int signed_distance(void)
{
	/* ascending line y = x - 4 */
	float2 p0 = { 2.0f, -2.0f };
	float2 p1 = { 6.0f, 2.0f };

	/* points */
	float2 left_of = { 3.0f, 1.0f };
	float2 right_of = { 5.0f, -1.0f };

	/* expected distance */
	double abs_dist = sqrt(2.0);

#define dist(x) line2d_dist(make_line2d(p0, p1), x)
	assert_near_eq(dist(left_of), -abs_dist);
	assert_near_eq(dist(right_of), abs_dist);
#undef dist
	return ok;
}

struct test const tests[] = {
	{ winding_order, "winding order of points" },
	{ test_in_circle, "points inside and outside a circle" },
	{ signed_distance, "signed distance from point to line" },

	{ NULL, NULL }
};
