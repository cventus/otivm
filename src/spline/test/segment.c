#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "ok/ok.h"
#include "gm/misc.h"
#include "gm/vector.h"
#include "base/mem.h"

#include "test.h"
#include "../geometry.h"

int test_create(void)
{
	struct lseg2d s;

	float2 from = { 2.f, 1.f }, to = { 1.f, 3.f };

	/* normal points right from tangent */
	double const n[2] = { 2.0 / sqrt(5.0), 1.0 / sqrt(5.0) };

	s = make_lseg2d(from, to);
	if (!v2neareq(s.l.n, n)) {
		printf("incorrect line normal\n");
		ok = -1;
	}

	return ok;
}

static void intersect(
	unsigned divisions,
	float min_angle,
	float min_scale,
	float max_scale,
	float x,
	float y)
{
	float const pi = acosf(0.f);

	unsigned i;
	float alpha, phi, theta, p0[2], p1[2], p2[2], p3[2], p4[2], p5[2];
	struct lseg2d s0, s1;

	for (i = 1; i < divisions; i++) {
		alpha = i * min_angle / 3.0;
		phi = alpha + min_angle + i*(2.0*pi - min_angle)/divisions;
		theta = alpha;

		Y[p0] = sinf(phi) * min_scale;
		X[p0] = cosf(phi) * min_scale;
		Y[p1] = -Y[p0];
		X[p1] = -X[p0];

		Y[p2] = sinf(theta) * max_scale;
		X[p2] = cosf(theta) * max_scale;
		Y[p3] = -Y[p2];
		X[p3] = -X[p2];

		Y[p0] += y;
		X[p0] += x;

		Y[p1] += y;
		X[p1] += x;

		Y[p2] += y;
		X[p2] += x;

		Y[p3] += y;
		X[p3] += x;

		s0 = make_lseg2d(p0, p1);
		s1 = make_lseg2d(p2, p3);

		if (!lseg2d_intersect(p4, s0, s1)) {
			printf("No intersection found %u\n", i);
			printf(" (%g, %g)->(%g, %g) @ %g\n", X[p0], Y[p0], X[p1], Y[p1], phi);
			printf(" (%g, %g)->(%g, %g) @ %g\n", X[p2], Y[p2], X[p3], Y[p3], theta);
			printf(" expected one at (%g, %g)\n", x, y);
			ok = -1;
			break;
		}
		if (!v2neareqef(p4, (float[2]){ x, y }, 1e-6, 1e-2)) {
			printf("expected intersection at (%g, %g),\n", x, y);
			printf("got (%g, %g)\n", X[p4], Y[p4]);
			ok = -1;
			break;
		}

		/* commutative */
		if (!lseg2d_intersect(p5, s1, s0)) {
			printf("No symmetric intersection found %u\n", i);
			printf(" (%g, %g)->(%g, %g) @ %g\n", X[p0], Y[p0], X[p1], Y[p1], phi);
			printf(" (%g, %g)->(%g, %g) @ %g\n", X[p2], Y[p2], X[p3], Y[p3], theta);
			printf(" expected one at (%g, %g)\n", x, y);
			ok = -1;
			break;
		}
		if (!v2neareqef(p5, (float[2]){ x, y }, 1e-6, 1e-2)) {
			printf("expected symmetric intersection at (%g, %g),\n", x, y);
			printf("got (%g, %g)\n", X[p5], Y[p5]);
			ok = -1;
			break;
		}

	}
}

int test_intersect_short_segments(void)
{
	intersect(600, 1e-3, 1e-4f, 1e-4f, 0.103f, 0.103f);
	return ok;
}

int test_intersect_short_and_long_segments(void)
{
	intersect(600, 1e-3, 1e-3f, 1e3f, 2.f, -2.f);
	return ok;
}

int test_intersect_long_segments(void)
{
	intersect(600, 1e-3, 1e4f, 1e4f, 42.f, 42.f);
	return ok;
}

int test_nonintersecting_end_points(void)
{
	struct lseg2d s0, s1;
	float p[2];
	size_t i, j, k;

	float2 polygon[] = {
		{ 0.0f, 1.1f },
		{ 1.0f, 1.0f },
		{ 2.0f, 2.0f },
		{ 4.0f, 3.0f },
		{ 4.0f, -3.0f },
		{ -0.0f, -2.0f }
	};

	for (i = 0; i < length_of(polygon); i++) {
		j = (i + 1) % length_of(polygon);
		k = (j + 1) % length_of(polygon);
		s0 = make_lseg2d(polygon[i], polygon[j]);
		s1 = make_lseg2d(polygon[j], polygon[k]);
		if (lseg2d_intersect(p, s0, s1)) {
			fail_test("end-points of s%zd and s%zd intersect at (%g, %g)\n", i, j, p[X], p[Y]);
		}
	}
	return ok;
}
