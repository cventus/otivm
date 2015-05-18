
#include <stdio.h>
#include <ok/ok.h>
#include <gm/matrix.h>
#include <float.h>

#include "../gen/generic.h"
#include "../gen/misc.g.h"
#include "../gen/matrix.g.h"

static int assert_equal(T const prod[static M*N], T const exp[static M*N])
{
	T diff[M*N];
	if (mneareq(prod, exp)) {
		return 0;
	} else {
		printf("Produced matrix:\n");
		mprint(prod);

		printf("Expected matrix:\n");
		mprint(exp);

		printf("Difference:\n");
		msube(diff, prod, exp);
		mprint(diff);
		ok = -1;
		return -1;
	}
}

static int assert_unequal(T const a[static M*N], T const b[static M*N])
{
	T diff[M*N];
	if (mneareq(a, b)) {
		printf("Matrices are incorrectly deemed equal!\n");
		mprint(a);
		printf("Difference:\n");
		msube(diff, a, b);
		mprint(diff);
		ok = -1;
		return -1;
	} else {
		return 0;
	}
}

static int test_id(void)
{
	int i, j;
	T mat[M*N], *p;

	p = mid(mat);
	for (i = 0; i < M; i++)
		for (j = 0; j < N; j++)
			if (!fneareq(*p++, (i == j) ? LIT(1.0) : LIT(0.0)))
				ok = -1;

	return ok;
}

static int test_zero(void)
{
	int i;
	T mat[M*N];

	mzero(mat);
	for (i = 0; i < M*N; i++)
		if (!fneareq(mat[i], LIT(0.0)))
			ok = -1;

	return ok;
}

static int test_eq(void)
{
	T mat[M*N], eps[M*N], rel[M*N], fuzzy[M*N];
	T const factor[] = { LIT(1.5), LIT(-1.5) };
	int i;

	/* Create random matrix */
	mrand(mat);

	/* Check exact equality */
	if (assert_equal(mat, mat)) {
		printf("A matrix should be equal to itself!\n");
	}

	/* Check fuzzy equality */
	for (i = 0; i < 2; i++) {
		/* Small additive scaling and noise should remain equal */
		mmuls(eps, mat, EQ_REL_EPSILON / factor[i]);
		madde(rel, mat, eps);
		madds(fuzzy, rel, EQ_ABS_EPSILON / factor[i]);
		if (assert_equal(mat, fuzzy)) {
			printf(
				"Not enough tolerance "
				"(relative: "PFMTG", absolute: "PFMTG")!\n",
				EQ_REL_EPSILON,
				EQ_ABS_EPSILON);
		}

		/* Large additive scaling and noise should make unequal */
		mmuls(eps, mat, EQ_REL_EPSILON * factor[i]);
		madde(fuzzy, mat, eps);
		madds(fuzzy, fuzzy, EQ_ABS_EPSILON * factor[i]);
		if (assert_unequal(mat, fuzzy)) {
			printf(
				"Too high tolerance "
				"(relative: "PFMTG", absolute: "PFMTG")!\n",
				EQ_REL_EPSILON,
				EQ_ABS_EPSILON);
		}
	}

	return ok;
}

static int test_copy(void)
{
	int i;
	T a[M*N], b[M*N], c[M*N];

	mid(a);
	mzero(b);
	for (i = 0; i < M*N; i++) { c[i] = i; }

	assert_unequal(a, b);
	assert_unequal(b, c);
	assert_unequal(c, a);

	mcopy(b, c);
	assert_equal(b, c);

	mcopy(a, c);
	assert_equal(a, c);

	return ok;
}

static int test_transp(void)
{
	int i, j;
	T mat[M*N], t[M*N], tt[M*N];

	for (i = 0; i < M*N; i++) { mat[i] = i; }

	mtranspose(t, mat);

	/* Check transposed property */
	for (i = 0; i < M; i++) {
		for (j = 0; j < N; j++) {
			ok = mref(mat, i, j) != mref(t, j, i);
		}
	}

	mtranspose(tt, t);
	assert_equal(tt, mat);

	return ok;
}

struct test tests[] = {
	{ test_id,	"produces the identity matrix" },
	{ test_zero,	"produces a zero matrix" },
	{ test_eq,	"matrix equality and inequality" },
	{ test_copy,	"copy a matrix" },
	{ test_transp,	"matrix transpose" },
};

size_t n_tests = sizeof tests / sizeof 0[tests];

