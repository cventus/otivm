#if (M != 3) || (N != 3)
#error "These functions are only defined for 3x3 matrices"
#endif

#include <math.h>
#include <assert.h>
#include <stdio.h>

#include "generic.h"
#include "array.g.h"

/* Declare MxN and 3x3 matrix functions */
#include "matrix.g.h"
#include "square-matrix.g.h"
#include "matrix33x.g.h"

/* Declare vector-3 functions */
#define L 3
#include "vector3x.g.h"
#define V3(name) VECTOR_(v3,name)

T mdet(T const a[static M*N])
{
	return MREF(a, 0, 0) * MREF(a, 1, 1) * MREF(a, 2, 2)
	     + MREF(a, 0, 1) * MREF(a, 1, 2) * MREF(a, 2, 0)
	     + MREF(a, 0, 2) * MREF(a, 1, 0) * MREF(a, 2, 1)
	     - MREF(a, 0, 2) * MREF(a, 1, 1) * MREF(a, 2, 0)
	     - MREF(a, 0, 1) * MREF(a, 1, 0) * MREF(a, 2, 2)
	     - MREF(a, 0, 0) * MREF(a, 1, 2) * MREF(a, 2, 1);
}

T mtrace(T const a[static M*N])
{
	T trace;
	int i;

	for (i = 0, trace = LIT(0.0); i < N; i++)
		trace += a[i*N + i];

	return trace;
}

T *minv(T dest[static M*N], T const A[static M*N])
{
	T det;

	det = mdet(A);
	assert(det != LIT(0.));

	return amuls(dest, M*N, mtranspose(dest, A), LIT(1.0)/det);
}

T * mpinv(T a[static M*N], T const b[static M*N])
{
	(void)a;
	(void)b;
	return 0;
}

T *mrotx(T a[static M*N], T rad) {
	T c, s;

	c = cosM(rad);
	s = sinM(rad);

	mid(a);
	a[3] = c;
	a[4] = s;
	a[6] = -s;
	a[8] = c;
	return a;
}

T *mroty(T a[static M*N], T rad)
{
	T c, s;

	c = cosM(rad);
	s = sinM(rad);

	mid(a);
	a[0] = c;
	a[2] = -s;
	a[6] = s;
	a[8] = c;
	return a;
}

T *mrotz(T a[static M*N], T rad)
{
	T c, s;

	c = cosM(rad);
	s = sinM(rad);

	mid(a);
	a[0] = c;
	a[1] = s;
	a[3] = -s;
	a[4] = c;
	return a;
}

T *mscale(T a[static M*N], T x, T y, T z)
{
	mzero(a);
	a[0] = x;
	a[4] = y;
	a[8] = z;
	return a;
}

T *muscale(T a[static M*N], T s)
{
	mzero(a);
	a[0] = s;
	a[4] = s;
	a[8] = s;
	return a;
}

T *
mmul(T a[static M*N], T const b [static M*N], T const c [static M*N])
{
	int i;
	for (i = 0; i < 3; i++) {
#define A(row,col) a[(col*3)+row]
#define B(row,col) b[(col*3)+row]
#define C(row,col) c[(col*3)+row]
		T const bi0=B(i,0), bi1=B(i,1), bi2=B(i,2);
		A(i,0) = bi0*C(0,0) + bi1*C(1,0) + bi2*C(2,0);
		A(i,1) = bi0*C(0,1) + bi1*C(1,1) + bi2*C(2,1);
		A(i,2) = bi0*C(0,2) + bi1*C(1,2) + bi2*C(2,2);
#undef A
#undef B
#undef C
	}
	return a;
}
