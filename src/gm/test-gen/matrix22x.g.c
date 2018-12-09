#include <stdio.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "ok/ok.h"
#include "gm/matrix.h"
#include "../gen/generic.h"
#include "../gen/matrix.g.h"
#include "../gen/square-matrix.g.h"
#include "../gen/matrix22x.g.h"
#include "../gen/misc.g.h"

#define lengthof(arr) (sizeof (arr) / sizeof 0[arr])

static int assert_equal(
	T const prod[static 4],
	T const exp[static 4],
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

static T const identity[2*2] = {
	LIT(1.0), LIT(0.0),
	LIT(0.0), LIT(1.0)
};

static T const col_counted[2*2] = {
	LIT(1.0), LIT(2.0),
	LIT(3.0), LIT(4.0)
};

static T const row_counted[2*2] = {
	LIT(1.0), LIT(3.0),
	LIT(2.0), LIT(4.0)
};

#define sin45 LIT(0.707106781186548)
#define cos45 LIT(0.707106781186548)

static T const rot_45[2*2] = {
	cos45,    sin45,
	-sin45,   cos45
};

static const T scale[2*2] = {
	LIT(3.0), LIT(0.0),
	LIT(0.0), LIT(5.0)
};

int test_determinant(void)
{
	static const T dmat[2*2] = {
		LIT(6.0), LIT(4.0),
		LIT(0.0), LIT(13.0)
	};

	static struct { const T (*matrix)[2*2], det; } const examples[] = {
		{ &identity, LIT(1.0) },
		{ &col_counted, LIT(-2.0) },
		{ &row_counted, LIT(-2.0) },
		{ &rot_45, LIT(1.0) },
		{ &scale, LIT(15.0) },
		{ &dmat, LIT(78.0) }
	};

	size_t i;
	T det, expected, tsp[2*2], inv[2*2], scalar[2*2];
	const T (*m)[2*2];

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
		if (!expect_equals(det, LIT(9.0)*expected, "%zd.3", i + 1)) {
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

int test_trace(void)
{
	static struct { const T (*matrix)[2*2], tr; } const examples[] = {
		{ &identity, LIT(2.0) },
		{ &col_counted, LIT(5.0) },
		{ &row_counted, LIT(5.0) },
		{ &rot_45, LIT(1.41421356237309) },
		{ &scale, LIT(8.0) },
	};

	size_t i;
	const T (*m)[2*2];
	T tr, expected;

	for (i = 0; i < lengthof(examples); i++) {
		m = examples[i].matrix;
		expected = examples[i].tr;

		tr = mtrace(*m);
		expect_equals(tr, expected, "%zd", i + 1);
	}

	return ok;
}

int test_invert_matrix(void)
{
	static const T (*invertible_matrices[])[2*2] = {
		&identity,
		&rot_45,
		&scale
	};

	size_t i;
	T inv[2*2], prod[2*2];
	const T (*m)[2*2];

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

int test_rotation_matrix(void)
{
	T mat[2*2];
	mrot(mat, acosM(LIT(-1.0))/4);
	assert_equal(mat, rot_45, 0);
	return ok;
}

int test_matrix_multiplication(void)
{
	T rot22[2*2], rot45[2*2];
	mrot(rot22, acosM(LIT(-1.0))/8);
	mmul(rot45, rot22, rot22);
	assert_equal(rot45, rot_45, 0);
	return ok;
}
