
#include <stdio.h>
#include <stdarg.h>
#include <ok/ok.h>
#include <gm/matrix.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "../gen/generic.h"
#include "../gen/matrix.g.h"
#include "../gen/square-matrix.g.h"
#include "../gen/matrix44x.g.h"
#include "../gen/misc.g.h"

#define lengthof(arr) (sizeof (arr) / sizeof 0[arr])

static int assert_equal(
	T const prod[static 16],
	T const exp[static 16],
	const char *message)
{
	T diff[16];
	if (!mneareq(prod, exp)) {
		if (message) puts(message);
		printf("Got:\n");
		mprint(prod);

		printf("Expected:\n");
		mprint(exp);

		printf("Difference:\n");
		msube(diff, prod, exp);
		mprint(diff);
		ok = -1;
		return -1;
	} else {
		return 0;
	}
}

static int vexpect_equalse(T val, T exp, T reps, T aeps, char const *fmt, va_list ap)
{
	if (!fneareqe(val, exp, reps, aeps)) {
		vprintf(fmt, ap);
		printf(": got "PFMTG", expected "PFMTG"\n", val, exp);
		ok = -1;
		return 0;
	} else {
		return 1;
	}
}

static int expect_equalse(T val, T exp, T reps, T aeps, char const *fmt, ...)
{
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = vexpect_equalse(val, exp, reps, aeps, fmt, ap);
	va_end(ap);
	return res;
}

static int expect_equals(T v, T e, char const *fmt, ...)
{
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = vexpect_equalse(v, e, EQ_REL_EPSILON, EQ_ABS_EPSILON, fmt, ap);
	va_end(ap);
	return res;
}

static T const identity[4*4] = {
	LIT(1.0), LIT(0.0), LIT(0.0), LIT(0.0),
	LIT(0.0), LIT(1.0), LIT(0.0), LIT(0.0),
	LIT(0.0), LIT(0.0), LIT(1.0), LIT(0.0),
	LIT(0.0), LIT(0.0), LIT(0.0), LIT(1.0),
};

static T const col_counted[4*4] = {
	LIT( 1.0), LIT( 2.0), LIT( 3.0), LIT( 4.0),
	LIT( 5.0), LIT( 6.0), LIT( 7.0), LIT( 8.0),
	LIT( 9.0), LIT(10.0), LIT(11.0), LIT(12.0),
	LIT(13.0), LIT(14.0), LIT(15.0), LIT(16.0),
};

static T const row_counted[4*4] = {
	LIT(1.0), LIT(5.0), LIT( 9.0), LIT(13.0),
	LIT(2.0), LIT(6.0), LIT(10.0), LIT(14.0),
	LIT(3.0), LIT(7.0), LIT(11.0), LIT(15.0),
	LIT(4.0), LIT(8.0), LIT(12.0), LIT(16.0),
};

#define sin45 LIT(0.707106781186548)
#define cos45 LIT(0.707106781186548)

static T const rot_y45[4*4] = {
	cos45,    LIT(0.0), -sin45,   LIT(0.0),
	LIT(0.0), LIT(1.0), LIT(0.0), LIT(0.0),
	sin45,    LIT(0.0), cos45,    LIT(0.0),
	LIT(0.0), LIT(0.0), LIT(0.0), LIT(1.0),
};

static T const rot_z45[4*4] = {
	cos45,    sin45,    LIT(0.0), LIT(0.0),
	-sin45,   cos45,    LIT(0.0), LIT(0.0),
	LIT(0.0), LIT(0.0), LIT(1.0), LIT(0.0),
	LIT(0.0), LIT(0.0), LIT(0.0), LIT(1.0),
};

static const T scale[4*4] = {
	LIT(3.0), LIT(0.0), LIT(0.0), LIT(0.0),
	LIT(0.0), LIT(5.0), LIT(0.0), LIT(0.0),
	LIT(0.0), LIT(0.0), LIT(-6.), LIT(0.0),
	LIT(0.0), LIT(0.0), LIT(0.0), LIT(1.0),
};

static int test_det(void)
{
	static const T dmat[4*4] = {
		LIT(6.0), LIT(4.0), LIT(-1.0), LIT(8.0),
		LIT(0.0), LIT(13.0), LIT(0.0), LIT(6.0),
		LIT(-3.0), LIT(6.0), LIT(7.0), LIT(0.0),
		LIT(5.0), LIT(-8.0), LIT(4.0), LIT(2.0),
	};

	static struct { const T (*matrix)[4*4], det; } const examples[] = {
		{ &identity, LIT(1.0) },
		{ &col_counted, LIT(0.0) },
		{ &row_counted, LIT(0.0) },
		{ &rot_z45, LIT(1.0) },
		{ &scale, LIT(-90.0) },
		{ &dmat, LIT(170.0) },
	};

	size_t i;
	T det, expected, tsp[4*4], inv[4*4], scalar[4*4];
	const T (*m)[4*4];

	for (i = 0; i < lengthof(examples); i++) {
		m = examples[i].matrix;
		expected = examples[i].det;

		/* 1. Check that the value is correct */
		det = mdet(*m);
		expect_equals(det, expected, "%zu.1", i + 1);

		/* 2. Determinant should be the same for the transpose */
		mtranspose(tsp, *m);
		det = mdet(tsp);
		expect_equals(det, expected, "%zu.2", i + 1);

		/* 3. Matrix multiplied by scalar property:
		      det(cA) = c^n det(A) */
		mmuls(scalar, *m, LIT(3.0));
		det = mdet(scalar);
		if (!expect_equals(det, LIT(81.0)*expected, "%zd.3", i + 1)) {
			mprint(scalar);
		}

		/* 4. The determinant of an inverse should be the same as the
		      inverse of the determinant */
		if (!fneareq(expected, LIT(0.0))) {
			T expinv = 1/expected;
			minv(inv, *m);
			det = mdet(inv);
			/* Allow a little more leniency here than usual with
			   a slightly larger absolute error epsilon. There
			   are many places where rounding errors can occur
			   up to this point. */
			expect_equalse(
				det,
				expinv,
				LIT(1e-6),
				LIT(1e-6),
				"%zd.4",
				i + 1);
		}
	}
	return ok;
}

static int test_trace(void)
{
	static struct { const T (*matrix)[4*4], tr; } const examples[] = {
		{ &identity, LIT(4.0) },
		{ &col_counted, LIT(34.0) },
		{ &row_counted, LIT(34.0) },
		{ &rot_y45, LIT(3.41421356237309) },
		{ &rot_z45, LIT(3.41421356237309) },
		{ &scale, LIT(3.0) },
	};

	size_t i;
	const T (*m)[4*4];
	T tr, expected;

	for (i = 0; i < lengthof(examples); i++) {
		m = examples[i].matrix;
		expected = examples[i].tr;

		tr = mtrace(*m);
		expect_equals(tr, expected, "%zd", i + 1);
	}

	return ok;
}

static int test_inv(void)
{
	static const T (*invertible_matrices[])[4*4] = {
		&identity,
		&rot_y45,
		&rot_z45,
		&scale
	};

	size_t i;
	T inv[4*4], prod[4*4];
	const T (*m)[4*4];

	for (i = 0; i < lengthof(invertible_matrices); i++) {
		m = invertible_matrices[i];
		minv(inv, *m);
		mmul(prod, inv, *m);
		assert_equal(prod, identity, "matrix x inverse");
		mmul(prod, *m, inv);
		assert_equal(prod, identity, "inverse x matrix");
	}

	return ok;
}

static int test_pinv(void)
{
	todo(0);
	return -1;
}

static int test_rotx(void)
{
	todo(0);
	return -1;
}

static int test_roty(void)
{
	T mat[4*4];
	mroty(mat, acosM(LIT(-1.0))/4);
	assert_equal(mat, rot_y45, 0);
	return ok;
}

static int test_rotz(void)
{
	T mat[4*4], pi = acosM(LIT(-1.0));
	mrotz(mat, pi/4);
	assert_equal(mat, rot_z45, 0);
	return ok;
}

static int test_trans(void)
{
	todo(0);
	return -1;
}

static int test_scale(void)
{
	todo(0);
	return -1;
}

static int test_uscale(void)
{
	todo(0);
	return -1;
}

static int test_mul(void)
{
	todo(0);
	return -1;
}

static int test_mulv(void)
{
	todo(0);
	return -1;
}

static int test_lookat(void)
{
	todo(0);
	return -1;
}

static int test_persp(void)
{
	todo(0);
	return -1;
}

static int test_quat(void)
{
	todo(0);
	return -1;
}


struct test tests[] = {
	{ test_det,	"determinant" },
	{ test_trace,	"trace" },
	{ test_inv,	"inverse" },
	{ test_pinv,	"pseudo-inverse" },
	{ test_rotx,	"make x-axis rotation matrix" },
	{ test_roty,	"make y-axis rotation matrix" },
	{ test_rotz,	"make z-axis rotation matrix" },
	{ test_trans,	"translation matrix" },
	{ test_scale,	"scale matrix" },
	{ test_uscale,	"uniform scale matrix" },
	{ test_mul,	"matrix multiplication" },
	{ test_mulv,	"transform a vector" },
	{ test_lookat,	"lookat matrix" },
	{ test_persp,	"perspective matrix" },
	{ test_quat,	"rotation matrix from quaternion" },
};

size_t n_tests = lengthof(tests);

