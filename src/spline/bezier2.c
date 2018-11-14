#include <stddef.h>
#include <string.h>
#include <math.h>

#include "spline/bezier2.h"

/* get the berstein polynomial terms evaluated at t */
static void bernstein2(double dest[3], double t)
{
	double s = 1.0 - t;
	dest[0] = s*s;
	dest[1] = 2*s*t;
	dest[2] = t*t;
}

float *bezier2(float *dest, size_t d, float const *p, double t)
{
	double c[3];
	float const *p0, *p1, *p2;
	size_t i;

	if (t <= 0.0) { return memcpy(dest, p, sizeof (float) * d); }
	if (t >= 1.0) { return memcpy(dest, p + 2*d, sizeof (float) * d); }

	p0 = p;
	p1 = p0 + d;
	p2 = p1 + d;
	bernstein2(c, t);
	for (i = 0; i < d; i++) {
		dest[i] = c[0]*p0[i] + c[1]*p1[i] + c[2]*p2[i];
	}
	return dest;
}

float *bezier2_split(float *dest, size_t d, float const *p, double t)
{
	double s;
	float const *p0, *p1, *p2;
	float *q1, *q2, *q3;
	size_t i;

	p0 = p;
	p1 = p0 + d;
	p2 = p1 + d;

	q1 = dest;
	q2 = q1 + d;
	q3 = q2 + d;

	if (t <= 0.0) {
		(void)memcpy(q1, p0, sizeof (float) * d);
		(void)memcpy(q2, p0, sizeof (float) * d);
		(void)memcpy(q3, p1, sizeof (float) * d);
		return dest;
	}
	if (t >= 1.0) {
		(void)memcpy(q1, p1, sizeof (float) * d);
		(void)memcpy(q2, p2, sizeof (float) * d);
		(void)memcpy(q3, p2, sizeof (float) * d);
		return dest;
	}

	s = 1.0 - t;
	for (i = 0; i < d; i++) {
		q1[i] = s*p0[i] + t*p1[i];
		q3[i] = s*p1[i] + t*p2[i];
		q2[i] = s*q1[i] + t*q3[i];
	}
	return dest;
}

float *rbezier2(float *dest, size_t d, float const *p, double t)
{
	size_t i;
	float a0, a1, a2, z;
	float const *p0, *p1, *p2;
	double c[3];

	if (t <= 0.0) { return memcpy(dest, p, sizeof (float) * d); }
	if (t >= 1.0) { return memcpy(dest, p + 2*(d+1), sizeof (float) * d); }

	bernstein2(c, t);

	/* point offsets */
	p0 = p;
	p1 = p0 + d + 1;
	p2 = p1 + d + 1;

	/* last component of each point is its weight */
	a0 = p0[d] * c[0];
	a1 = p1[d] * c[1];
	a2 = p2[d] * c[2];

	/* total of weights and terms */
	z = a0 + a1 + a2;
	for (i = 0; i < d; i++) {
		dest[i] = (a0*p0[i] + a1*p1[i] + a2*p2[i])/z;
	}

	return dest;
}

float rbezier2_norm_w1(float w0, float w1, float w2)
{
	return sqrtf((w1*w1)/(w0*w2));
}
