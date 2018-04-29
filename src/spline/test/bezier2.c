
#include <stdio.h>
#include <stddef.h>
#include <math.h>

#include <ok/ok.h>
#include <gm/misc.h>

#include "../include/bezier2.h"

static int endpoints(void)
{
	float points[] = {
		13.0f, 37.0f,
		10.24f, 20.48f,
		1.0f, 1.0f
	};
	float p[2], err, maxerr;

	maxerr = 1e-6;

	bezier2(p, 2, points, 0.0);
	err = (p[0] - points[0]) + (p[1] - points[1]);
	if (err > maxerr) {
		fail_test("error too great (%f) at t=0.0\n", err);
	}

	bezier2(p, 2, points, 1.0);
	err = (p[0] - points[4]) + (p[1] - points[5]);
	if (err > maxerr) {
		fail_test("error too great (%f) at t=1.0\n", err);
	}

	return ok;
}

static int parabola(void)
{
	/* these points define a bézier spline where <P0,P1> and <P1,P2> are
	   tangent to the curve y = x^2 and x also goes linearly from 0 to 1 as
	   t goes from 0 to 1 */
	float points[] = {
		0.0f, 0.0f,
		0.5f, 0.0f,
		1.0f, 1.0f
	};
	float p[2], x, y, err, maxerr;
	size_t i;

	maxerr = 1e-6;

	for (i = 0; i <= 10000; i++) {
		x = i / 10000.0;
		bezier2(p, 2, points, x);

		err = p[0] - x;
		if (fabsf(err) > maxerr) {
			fail_test("x error %f too great at t=%f\n", err, x);
		}
		y = x*x;
		err = p[1] - y;
		if (fabsf(err) > maxerr) {
			fail_test("y error %f too great at t=%f\n", err, x);
		}
	}
	return ok;
}

static int conic_circle(void)
{
	/* Quadratic rational Bézier splines can represent any conic section,
	   for instance circles. The following points represent a 90 degree
	   circular arc. */
	float points[] = {
		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0/sqrt(2.0),
		0.0f, 1.0f, 1.0f
	};
	float p[2], rel_err, abs_err;
	double t, half_pi, x, y;
	size_t i, n;

	n = 10000;
	rel_err = .025f; /* 2.5% relative error */
	abs_err = 0.01f; /* combined with 1/100 absolute error */
	half_pi = asin(1.0);

	for (i = 0; i <= n; i++) {
		t = i / (double)n;
		rbezier2(p, 2, points, t);

		x = cos(t * half_pi);
		if (!fneareqef(p[0], x, rel_err, abs_err)) {
			printf("x=%f\tp[0]=%f: %f\n", x, p[0], p[0] - x);
			ok = -1;
		}

		y = sin(t * half_pi);
		if (!fneareqef(p[1], y, rel_err, abs_err)) {
			printf("y=%f\tp[1]=%f: %f\n", y, p[1], p[1] - y);
			ok = -1;
		}
	}
	return ok;
}

static int split(void)
{
	float points[] = {
		1.0f, 1.0f,
		1.0f, 2.0f,
		2.0f, 2.0f
	};
	float expected_points[] = {
		1.0f, 1.5f,
		1.5f, 1.5f,
		1.5f, 2.0f
	};
	float pt[6], err, maxerr;
	size_t i;

	bezier2_split(pt, 2, points, 0.5);

	maxerr = 1e-6;

	err = 0.0;
	for (i = 0; i < 6; i++) {
		err += pt[i] - expected_points[i];
	}
	if (err > maxerr) {
		fail_test("error too great (%f) at t=0.0\n", err);
	}

	return ok;
}

struct test const tests[] = {
	{ endpoints, "interpolate endpoints at t=0 and t=1" },
	{ parabola, "compare bezier evaluation function to parabola" },
	{ conic_circle, "quarter circle segment with rational bezier curve" },
	{ split, "subdivide a quadratic Bézier curve" },

	{ NULL, NULL }
};

