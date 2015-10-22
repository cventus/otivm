#include "generic.h"
#include "plane.g.h"

#define L 3
#include "vector.g.h"
#include "vector3x.g.h"

#include <math.h>
#include <float.h>
#include <assert.h>

T *plane_normal(T normal[static 3], const T p0[static 3], const T p1[static 3], const T p2[static 3])
{
	T n[3], v01[3], v02[3];

	return vnorm(normal, vcross(n, vsub(v01, p1, p0), vsub(v02, p2, p0)));
}

int defines_plane(const T p0[static 3], const T p1[static 3], const T p2[static 3])
{
	T n[3], v01[3], v02[3], zero[3] = { LIT(0.0), LIT(0.0), LIT(0.0) };

	if (vneareq(zero, vcross(n, vsub(v01, p1, p0), vsub(v02, p2, p0)))) {
		return 0;
	}
	return 1;
}

