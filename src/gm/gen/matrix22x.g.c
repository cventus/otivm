#if (M != 2) || (N != 2)
#error "These functions are only defined for 3x3 matrices"
#endif

#include <math.h>
#include <assert.h>
#include <stdio.h>

#include "generic.h"

/* Declare MxN and 2x2 matrix functions */
#include "matrix.g.h"
#include "square-matrix.g.h"
#include "matrix22x.g.h"

T mdet(T const a[static M*N])
{
	return a[0]*a[3] - a[1]*a[2];
}

T mtrace(T const a[static M*N])
{
	return a[0] + a[3];
}

T *minv(T dest[static M*N], T const A[static M*N])
{
	T det, invdet;

	det = mdet(A);
	assert(det != LIT(0.0));
	invdet = LIT(1.0)/det;

	dest[0] = A[3] * invdet;
	dest[1] =-A[1] * invdet;
	dest[2] =-A[2] * invdet;
	dest[3] = A[0] * invdet;

	return dest;
}

T *mrot(T a[static M*N], T rad) {
	T c, s;

	c = cosM(rad);
	s = sinM(rad);

	a[0] = c;
	a[1] = s;
	a[2] = -s;
	a[3] = c;
	return a;
}

T *mshearx(T a[static M*N], T s)
{
	mid(a);
	a[2] = s;
	return a;
}

T *msheary(T a[static M*N], T s)
{
	mid(a);
	a[1] = s;
	return a;
}

T *mscale(T a[static M*N], T x, T y)
{
	mzero(a);
	a[0] = x;
	a[3] = y;
	return a;
}

T *muscale(T a[static M*N], T s)
{
	mzero(a);
	a[0] = s;
	a[3] = s;
	return a;
}

T *
mmul(T a[restrict static M*N], T const b [static M*N], T const c [static M*N])
{
	a[0] = b[0]*c[0] + b[2]*c[1];
	a[1] = b[1]*c[0] + b[3]*c[1];
	a[2] = b[0]*c[2] + b[2]*c[3];
	a[3] = b[1]*c[2] + b[3]*c[3];
	return a;
}
