#if L != 3
#error "Cross-product only defined for vectors of length 3"
#endif
#ifndef GENERIC_H
#include "generic.h"
#endif
#include "vector.h"
#include "vector.g.c"

T *
vcross(T w[static L], T const u[static L], T const v[static L])
{
	T u1v2, u2v1, u2v0, u0v2, u0v1, u1v0;

	u1v2 = u[1]*v[2];
	u2v1 = u[2]*v[1];
	u2v0 = u[2]*v[0];
	u0v2 = u[0]*v[2];
	u0v1 = u[0]*v[1];
	u1v0 = u[1]*v[0];

	w[0] = u1v2 - u2v1;
	w[1] = u2v0 - u0v2;
	w[2] = u0v1 - u1v0;

	return w;
}

