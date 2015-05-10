#ifndef GENERIC_H
#include "generic.h"
#endif
#ifndef VECTOR_H
#include "vector.h"
#endif

#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>

#include "vector.g.h"
#include "array.g.h"
#include "misc.g.h"

T *vcopy(T w[static L], const T u[static L])
{
	memcpy(w, u, sizeof (T [L]));
	return w;
}

T *vzero(T w[static L])
{
	int i;
	for (i = 0; i < L; i++) {
		w[i] = LIT(0.0);
	}
	return w;
}

T *
vneg(T w[static L], const T u[static L])
{
	int i;
	for (i = 0; i < L; i++) {
		w[i] = -u[i];
	}
	return w;
}

T *vadd(T w[static L], const T u[static L], const T v[static L])
{
	int i;
	T tmp[L];

	for (i = 0; i < L; i++) {
		tmp[i] = u[i] + v[i];
	}
	return vcopy(w, tmp);
}

T *vsub(T w[static L], const T u[static L], const T v[static L])
{
	T tmp[L];
	return vadd(w, u, vneg(tmp, v));
}

T vdot(const T u[static L], const T v[static L])
{
	int i;
	T sum;

	sum = LIT(0.0);
	for (i = 0; i < L; i++) {
		sum += u[i]*v[i];
	}
	return sum;
}

T vlen2(const T u[static L])
{
	return vdot(u, u);
}

T vlen(const T u[static L])
{
	return sqrtM(vlen2(u));
}

T *vscale(T w[static L], const T u[static L], T s)
{
	int i;
	for (i = 0; i < L; i++) {
		w[i] = s * u[i];
	}
	return w;
}

T *vnorm(T w[static L], const T u[static L])
{
	return vscale(w, u, invsqrt(vlen2(u)));
}

int vtrynorm(T w[static L], const T u[static L])
{
	T len = vlen2(u);
	if (fneareq(len, LIT(0.0))) { return -1; }
	(void)vscale(w, u, invsqrt(len));
	return 0;
}

int vneareqe(const T u[static L], const T v[static L], T rel_e, T abs_e)
{
	return aneareqe(u, v, L, rel_e, abs_e);
}

int vneareq(const T u[static L], const T v[static L])
{
	return vneareqe(u, v, EQ_REL_EPSILON, EQ_ABS_EPSILON);
}

