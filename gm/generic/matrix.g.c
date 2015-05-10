
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "generic.h"

#include "array.g.h"
#include "misc.g.h"
#include "matrix.h"
#include "matrix.g.h"

T *mcopy(T dest[static M*N], const T src[static M*N])
{
	return memcpy(dest, src, sizeof (T [M*N]));
}

static T rnd(T init)
{
	(void)init;
	return frand();
}

T *mrand(T dest[static M*N])
{
	amapu(dest, M*N, dest, &rnd);
	return dest;
}

T *mzero(T a[static M*N])
{
	int i;
	for (i = 0; i < M*N; i++) {
		a[i] = LIT(0.);
	}
	return a;
}

T *mid(T a[static M*N])
{
	return msid(a, LIT(1.));
}

T *msid(T a[static M*N], T s)
{
	int i;

	mzero(a);
	for (i = 0; i < DIAGONAL; i++) {
		a[i + i*M] = s;
	}
	return a;
}

T *mtranspose(T a[static M*N], T const b[static M*N])
{
	int i, j;

	for (i = 0; i < M; i++) {
		for (j = 0; j < N; j++) {
			a[i*M + j] = b[j*M + i];
		}
	}

	return a;
}

T *madde(T dest[M*N], T const a[static M*N], T const b[static M*N])
{
	return aadd(dest, M*N, a, b);
}

T *madds(T dest[M*N], T const a[static M*N], T b)
{
	return aadds(dest, M*N, a, b);
}

T *msube(T dest[M*N], T const a[static M*N], T const b[static M*N])
{
	return asub(dest, M*N, a, b);
}

T *msubs(T dest[M*N], T const a[static M*N], T b)
{
	return asubs(dest, M*N, a, b);
}

T *mmule(T dest[M*N], T const a[static M*N], T const b[static M*N])
{
	return amul(dest, M*N, a, b);
}

T *mmuls(T dest[M*N], T const a[static M*N], T b)
{
	return amuls(dest, M*N, a, b);
}

T *mdive(T dest[M*N], T const a[static M*N], T const b[static M*N])
{
	return adiv(dest, M*N, a, b);
}

T *mdivs(T dest[M*N], T const a[static M*N], T b)
{
	return adivs(dest, M*N, a, b);
}

T mref(T const a[static M*N], unsigned row, unsigned column)
{
	return a[column * N + row];
}

T mmse(T const a[static M*N], T const b[static M*N])
{
	T e, se;
	unsigned i;

	for (se = LIT(0.), i = 0; i < M*N; i++) {
		e = a[i] - b[i];
		se += e*e;
	}

	return se / (T)(M*N);
}

void mfprint(FILE *fp, T const a[static M*N])
{
	int i, j, w, colw[N];

	memset(colw, 0, sizeof colw);
	for (j = 0; j < N; j++) {
		for (i = 0; i < M; i++) {
			w = snprintf(NULL, 0, PFMTG, mref(a, i, j));
			if (w > colw[j]) { colw[j] = w; }
		}
	}

	fputs("┌", fp);
	for (j = 0; j < N; j++) {
		for (i = 0; i < colw[j] + 2; i++) { fputs(" ", fp); }
	}
	fputs( "┐\n", fp);

	for (i = 0; i < M; i++) {
		fputs("│", fp);
		for (j = 0; j < N; j++) {
			fprintf(fp, " " PFMTGW("*") " ", colw[j],
			        mref(a, i, j));
		}
		fputs("│\n", fp);
	}

	fputs("└", fp);
	for (j = 0; j < N; j++) {
		for (i = 0; i < colw[j] + 2; i++) { fputs(" ", fp); }
	}
	fputs( "┘\n", fp);
}

void mprint(T const a[static M*N])
{
	mfprint(stdout, a);
}

int mneareqe(T const a[static M*N], T const b[static M*N], T rel_e, T abs_e)
{
	return aneareqe(a, b, M*N, rel_e, abs_e);
}

int mneareq(T const a[static M*N], T const b[static M*N])
{
	return mneareqe(a, b, EQ_REL_EPSILON, EQ_ABS_EPSILON);
}

