#include <math.h>
#include "generic.h"

/* Include vector-4 function declarations (common for quaternion) */
#include "vector.g.h"
#include "quaternionx.g.h"

/* Include vector-3 function declarations */
#undef L
#define L 3
#undef VECTOR_TAG
#define VECTOR_TAG v3
#include "vector.g.h"
#include "vector3x.g.h"
#define V3(name) VECTOR_(v3,name)

T *qid(T dest[static 4])
{
	dest[0] = LIT(1.0);
	dest[1] = LIT(0.0);
	dest[2] = LIT(0.0);
	dest[3] = LIT(0.0);
	return dest;
}

T *qaxisangle(T dest[static 4], T angle, T const normal_axis[static 3])
{
	T ha, sin_ha;

	ha = angle * LIT(0.5);
	sin_ha = sinM(ha);

	dest[0] = cosM(ha);
	dest[1] = sin_ha * normal_axis[0];
	dest[2] = sin_ha * normal_axis[1];
	dest[3] = sin_ha * normal_axis[2];

	return dest;
}

T *qslerpw(T dest[static 4], T const q0[static 4], T const q1[static 4],
             T omega, T t)
{
	T q0t[4], q1t[4], sum[4];

	qscale(q0t, q0, sinM(omega * (LIT(1.0) - t)));
	qscale(q1t, q1, sinM(omega * t));
	
	qadd(sum, q0t, q1t);
	return qscale(dest, sum, LIT(1.0)/sinM(omega));
}

T *qslerp(T dest[static 4], T const q0[static 4], T const q1[static 4], T t)
{
	return qslerpw(dest, q0, q1, acosM(qdot(q0, q1)), t);
}

T *qmul(T dest[static 4], T const q0[static 4], T const q1[static 4])
{
	T a[3], b[3], c[3], dot;
	V3(scale)(a, q1 + 1, q0[0]);
	V3(scale)(b, q0 + 1, q1[0]);
	V3(add)(a, a, b);
	V3(cross)(c, q0 + 1, q1 + 1);
	dot = V3(dot)(q0 + 1, q1 + 1);
	dest[0] = q0[0]*q1[0] - dot;
	V3(add)(dest + 1, a, c);
	return dest;
}

T *qconj(T dest[static 4], T const q[static 4])
{
	dest[0] = q[0];
	V3(neg)(dest + 1, q + 1);
	return dest;
}

T *
rotate(T dest[static 3], T const u[static 3], T const q[static 4])
{
	T conj[4], v[4], w[4];
	v[0] = LIT(0.0);
	V3(copy)(v + 1, u);
	qconj(conj, q);
	qmul(w, q, v);
	qmul(v, w, conj);
	return qcopy(dest, v + 1);
}

