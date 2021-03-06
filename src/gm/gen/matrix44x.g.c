#if (M != 4) || (N != 4)
#error "These functions are only defined for 4x4 matrices"
#endif

#include <math.h>
#include <assert.h>
#include <stdio.h>

#include "generic.h"
#include "array.g.h"

/* Declare MxN and 4x4 matrix functions */
#include "matrix.g.h"
#include "square-matrix.g.h"
#include "matrix44x.g.h"

/* Declare 3x3 matrix functions */
#undef M
#undef N
#define M 3
#define N 3
#include "matrix.g.h"
#include "square-matrix.g.h"
#include "matrix33x.g.h"

/* Restore M and N*/
#undef M
#undef N
#define M 4
#define N 4

/* Declare vector functions */
#define L 3
#include "vector3x.g.h"
#define V3(name) VECTOR_(v3,name)

T mdet(T const a[static M*N])
{
#define DET3x3(c0, c1, c2) \
	( MREF(a, c0, 1) * MREF(a, c1, 2) * MREF(a, c2, 3) \
	+ MREF(a, c0, 2) * MREF(a, c1, 3) * MREF(a, c2, 1) \
	+ MREF(a, c0, 3) * MREF(a, c1, 1) * MREF(a, c2, 2) \
	- MREF(a, c0, 3) * MREF(a, c1, 2) * MREF(a, c2, 1) \
	- MREF(a, c0, 1) * MREF(a, c1, 3) * MREF(a, c2, 2) \
	- MREF(a, c0, 2) * MREF(a, c1, 1) * MREF(a, c2, 3) )

	const T x = mref(a, 0, 0) * DET3x3(1, 2, 3);
	const T y = mref(a, 0, 1) * DET3x3(0, 2, 3);
	const T z = mref(a, 0, 2) * DET3x3(0, 1, 3);
	const T w = mref(a, 0, 3) * DET3x3(0, 1, 2);

	return x - y + z - w;
#undef DET3x3
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
	T A2[M*N], A3[M*N], a[M*N], b[M*N], c[M*N], ca, cb;
	T det, trA, trA2, trAtrA;

	/* determinant */
	det = mdet(A);
	assert(det != LIT(0.));

	/* square and cube */
	mmul(A2, A, A);
	mmul(A3, A2, A);

	/* trace factors */
	trA = mtrace(A);
	trA2 = mtrace(A2);
	trAtrA = trA*trA;

	ca = trA*trAtrA - LIT(3.0)*trA*trA2 + LIT(2.0)*mtrace(A3);
	msid(a, ca * (LIT(1.0)/LIT(6.0)));

	cb = trAtrA - trA2;
	amuls(b, M*N, A, cb * LIT(0.5));

	amuls(c, M*N, A2, trA);

	asub(dest, M*N, c, A3);
	aadd(dest, M*N, dest, a);
	asub(dest, M*N, dest, b);

	return amuls(dest, M*N, dest, LIT(1.0)/det);
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
	a[5] = c;
	a[6] = s;
	a[9] = -s;
	a[10] = c;
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
	a[8] = s;
	a[10] = c;
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
	a[4] = -s;
	a[5] = c;
	return a;
}

T *mtranslate(T a[static M*N], T x, T y, T z)
{
	mid(a);
	a[12] = x;
	a[13] = y;
	a[14] = z;
	return a;
}

T *mscale(T a[static M*N], T x, T y, T z)
{
	mzero(a);
	a[0] = x;
	a[5] = y;
	a[10] = z;
	a[15] = LIT(1.0);
	return a;
}

T *muscale(T a[static M*N], T s)
{
	mzero(a);
	a[0] = s;
	a[5] = s;
	a[10] = s;
	a[15] = LIT(1.0);
	return a;
}

T *
mmul(T a[static M*N], T const b [restrict static M*N], T const c [restrict static M*N])
{
	for (size_t i = 0; i < 4; i++) {
#define A(row,col) a[(col<<2)+row]
#define B(row,col) b[(col<<2)+row]
#define C(row,col) c[(col<<2)+row]
		T const bi0 = B(i,0), bi1 = B(i,1), bi2 = B(i,2), bi3 = B(i,3);
		A(i,0) = bi0*C(0,0) + bi1*C(1,0) + bi2*C(2,0) + bi3*C(3,0);
		A(i,1) = bi0*C(0,1) + bi1*C(1,1) + bi2*C(2,1) + bi3*C(3,1);
		A(i,2) = bi0*C(0,2) + bi1*C(1,2) + bi2*C(2,2) + bi3*C(3,2);
		A(i,3) = bi0*C(0,3) + bi1*C(1,3) + bi2*C(2,3) + bi3*C(3,3);
#undef A
#undef B
#undef C
	}
	return a;
}

T *
mmulv(T dest[static M*1], T const m44 [restrict static M*N], T const x [restrict static M*1])
{
	T const x0 = x[0], x1 = x[1], x2 = x[2], x3 = x[3];
#define A(row,col) m44[(col<<2)+row]
	dest[0] = x0*A(0,0) + x1*A(0,1) + x2*A(0,2) + x3*A(0,3);
	dest[1] = x0*A(1,0) + x1*A(1,1) + x2*A(1,2) + x3*A(1,3);
	dest[2] = x0*A(2,0) + x1*A(2,1) + x2*A(2,2) + x3*A(2,3);
	dest[3] = x0*A(3,0) + x1*A(3,1) + x2*A(3,2) + x3*A(3,3);
#undef A
	return dest;
}

T *
mmulv3(T dest[static 3*1], T const m44 [restrict static M*N], T const x [restrict static 3*1])
{
	T const x0 = x[0], x1 = x[1], x2 = x[2];
#define A(row,col) m44[(col<<2)+row]
	dest[0] = x0*A(0,0) + x1*A(0,1) + x2*A(0,2) + A(0,3);
	dest[1] = x0*A(1,0) + x1*A(1,1) + x2*A(1,2) + A(1,3);
	dest[2] = x0*A(2,0) + x1*A(2,1) + x2*A(2,2) + A(2,3);
#undef A
	return dest;
}

T *
mlookat(T a[static M*N], T const eye [static 3], T const center [static 3],
       T const up [static 3])
{
	T v[3], u[3], f[3], s[3];

	/* f = |center - eye| */
	V3(sub)(v, center, eye);
	V3(norm)(f, v);

	/* s = |f x |up|| */
	V3(norm)(u, up);
	V3(cross)(v, f, u);
	V3(norm)(s, v);

	/* u = s x f */
	V3(cross)(u, s, f);

	/* first row */
	a[0*4 + 0] = s[0];
	a[1*4 + 0] = s[1];
	a[2*4 + 0] = s[2];
	a[3*4 + 0] = -V3(dot)(s, eye);
	/* second row */
	a[0*4 + 1] = u[0];
	a[1*4 + 1] = u[1];
	a[2*4 + 1] = u[2];
	a[3*4 + 1] = -V3(dot)(u, eye);
	/* third row */
	a[0*4 + 2] = -f[0];
	a[1*4 + 2] = -f[1];
	a[2*4 + 2] = -f[2];
	a[3*4 + 2] =  V3(dot)(f, eye);
	/* fourth row */
	a[0*4 + 3] = LIT(0.0);
	a[1*4 + 3] = LIT(0.0);
	a[2*4 + 3] = LIT(0.0);
	a[3*4 + 3] = LIT(1.0);

	return a;
}

T *mperspective(T a[static M*N], T fovy, T aspect, T znear, T zfar)
{
	T tan_half_fov, neg_inv_zrange;

	assert(aspect > LIT(0.0));
	assert(LIT(0.0) < znear);
	assert(znear < zfar);
	assert(fovy < 2.0*acos(-1.0));

	tan_half_fov = tanM(fovy * LIT(0.5));
	if (aspect < 1.f) {
		tan_half_fov /= aspect;
	}

	neg_inv_zrange = 1 / (znear - zfar);

	a[ 0] = LIT(1.0) / (aspect * tan_half_fov);
	a[ 1] = LIT(0.0);
	a[ 2] = LIT(0.0);
	a[ 3] = LIT(0.0);

	a[ 4] = LIT(0.0);
	a[ 5] = LIT(1.0) / tan_half_fov;
	a[ 6] = LIT(0.0);
	a[ 7] = LIT(0.0);

	a[ 8] = LIT(0.0);
	a[ 9] = LIT(0.0);
	a[10] = (zfar + znear) * neg_inv_zrange;
	a[11] = LIT(-1.0);

	a[12] = LIT(0.0);
	a[13] = LIT(0.0);
	a[14] = LIT(2.0) * znear * zfar * neg_inv_zrange;
	a[15] = LIT(0.0);

	return a;
}

T *morthographic(
	T a[static M*N],
	T left,
	T right,
	T bottom,
	T top,
	T near,
	T far)
{
	assert(left != right);
	assert(top != bottom);
	assert(near != far);

	a[ 0] = LIT(2.0) / (right - left);
	a[ 1] = LIT(0.0);
	a[ 2] = LIT(0.0);
	a[ 3] = LIT(0.0);

	a[ 4] = LIT(0.0);
	a[ 5] = LIT(2.0) / (top - bottom);
	a[ 6] = LIT(0.0);
	a[ 7] = LIT(0.0);

	a[ 8] = LIT(0.0);
	a[ 9] = LIT(0.0);
	a[10] = LIT(-2.0) / (far - near);
	a[11] = LIT(0.0);

	a[12] = -(right + left) / (right - left);
	a[13] = -(top + bottom) / (top - bottom);
	a[14] = -(far + near) / (far - near);
	a[15] = LIT(1.0);

	return a;
}

T *mquat(T dest[static M*N], T const q[static 4]) {
	T q0, q1, q2, q3;

	q0 = q[0];
	q1 = q[1];
	q2 = q[2];
	q3 = q[3];

	dest[ 0] = LIT(1.0) - LIT(2.0)*(q2*q2 + q3*q3);
	dest[ 1] = LIT(2.0)*(q1*q2 + q3*q0);
	dest[ 2] = LIT(2.0)*(q1*q3 - q2*q0);
	dest[ 3] = LIT(0.0);

	dest[ 4] = LIT(2.0)*(q1*q2 - q3*q0);
	dest[ 5] = LIT(1.0) - LIT(2.0)*(q1*q1 + q3*q3);
	dest[ 6] = LIT(2.0)*(q1*q0 + q2*q3);
	dest[ 7] = LIT(0.0);

	dest[ 8] = LIT(2.0)*(q1*q3 + q2*q0);
	dest[ 9] = LIT(2.0)*(q2*q3 - q1*q0);
	dest[10] = LIT(1.0) - LIT(2.0)*(q1*q1 + q2*q2);
	dest[11] = LIT(0.0);

	dest[12] = LIT(0.0);
	dest[13] = LIT(0.0);
	dest[14] = LIT(0.0);
	dest[15] = LIT(1.0);

	return dest;
}
