#include <math.h>
#include "geometry.h"

double line2d_det(float2 a, float2 b, float2 c)
{
	double p = X[a]*Y[b] + X[b]*Y[c] + X[c]*Y[a];
	double n = X[c]*Y[b] + X[b]*Y[a] + X[a]*Y[c];
	return p - n;
}

struct line2d make_line2d(float2 p0, float2 p1)
{
	double dx, dy, a, b, c, s;

	dy = Y[p1] - Y[p0];
	dx = X[p0] - X[p1];
	s = hypot(dx, dy);
	a = dy / s;
	b = dx / s;
	c = -(X[p1]*a + Y[p1]*b);

	return (struct line2d){ { a, b }, c };
}

_Bool point2d_in_circle(float2 a, float2 b, float2 c, float2 d)
{
	/* point d within circle specified by (a, b, c), if

		| X[a]-X[d]  Y[a]-Y[d]  ((X[a]-X[d])^2 + (Y[a]-Y[d])^2) |
		| X[b]-X[d]  Y[b]-Y[d]  ((X[b]-X[d])^2 + (Y[b]-Y[d])^2) | > 0
		| X[c]-X[d]  Y[c]-Y[d]  ((X[c]-X[d])^2 + (Y[c]-Y[d])^2) |
	*/
	double dx0 = X[a] - X[d];
	double dx1 = X[b] - X[d];
	double dx2 = X[c] - X[d];

	double dy0 = Y[a] - Y[d];
	double dy1 = Y[b] - Y[d];
	double dy2 = Y[c] - Y[d];

	double d20 = dx0*dx0 + dy0*dy0;
	double d21 = dx1*dx1 + dy1*dy1;
	double d22 = dx2*dx2 + dy2*dy2;

	double p0 = dx0 * dy1 * d22;
	double p1 = dx2 * dy0 * d21;
	double p2 = dx1 * dy2 * d20;

	double n0 = dx2 * dy1 * d20;
	double n1 = dx0 * dy2 * d21;
	double n2 = dx1 * dy0 * d22;

	return p0 - n0 + p1 - n1 + p2 - n2 > EPSILON;
}

/* triangle defined by the positive side of three lines */
_Bool point2d_in_triangle(struct line2d const tri[3], float2 p)
{
	return line2d_dist(tri[0], p) >= -EPSILON &&
		line2d_dist(tri[1], p) >= -EPSILON &&
		line2d_dist(tri[2], p) >= -EPSILON;
}
