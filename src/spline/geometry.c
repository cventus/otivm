#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <gm/vector.h>
#include "geometry.h"

double line2d_det(float2 a, float2 b, float2 c)
{
	return X[a]*Y[b] - X[c]*Y[b] + X[b]*Y[c] - X[b]*Y[a] + X[c]*Y[a] - X[a]*Y[c];
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

bool point2d_in_circle(float2 a, float2 b, float2 c, float2 d)
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
bool point2d_in_triangle(struct line2d const tri[3], float2 p)
{
	return line2d_dist(tri[0], p) >= -EPSILON &&
		line2d_dist(tri[1], p) >= -EPSILON &&
		line2d_dist(tri[2], p) >= -EPSILON;
}

float *line2d_intersect(float dest[2], struct line2d l0, struct line2d l1)
{
	 double det;

	 det = X[l1.n]*Y[l0.n] - X[l0.n]*Y[l1.n];
	 if (fabs(det) >= EPSILON) {
		X[dest] = -(Y[l0.n]*l1.c - l0.c*Y[l1.n])/det;
		Y[dest] = -(X[l1.n]*l0.c - l1.c*X[l0.n])/det;
		return dest;
	 } else {
		return NULL;
	 }
}

/* unit vector parallel with line */
static void line2d_vector(float dest[2], struct line2d line)
{
	X[dest] = Y[line.n];
	Y[dest] = -X[line.n];
}

struct lseg2d make_lseg2d(float2 p0, float2 p1)
{
	struct lseg2d s;
	float r[XY];

	s.l = make_line2d(p0, p1);
	line2d_vector(r, s.l);
	(void)memcpy(s.p0, p0, sizeof s.p0);
	(void)memcpy(s.p1, p1, sizeof s.p1);
	return s;
}

float *lseg2d_p0(float dest[2], struct lseg2d s)
{
	return memcpy(dest, s.p0, sizeof s.p0);
}

float *lseg2d_p1(float dest[2], struct lseg2d s)
{
	return memcpy(dest, s.p0, sizeof s.p0);
}

static bool in_bounding_box(struct lseg2d s, float2 p)
{
	return
		((s.p0[0] < p[0] && p[0] < s.p1[0]) ||
		(s.p0[0] > p[0] && p[0] > s.p1[0])) &&
		((s.p0[1] < p[1] && p[1] < s.p1[1]) ||
		(s.p0[1] > p[1] && p[1] > s.p1[1]));
}

float *lseg2d_intersect(float dest[2], struct lseg2d s0, struct lseg2d s1)
{
	if (line2d_intersect(dest, s0.l, s1.l)) {
		/* intersecting lines */
		if (in_bounding_box(s0, dest) && in_bounding_box(s1, dest)) { return dest; }
	} else if (fabs(line2d_dist(s0.l, s1.p0)) < EPSILON) {
		/* parallel, or nearly parallel lines */
		if (in_bounding_box(s0, s1.p0)) { return dest; }
		if (in_bounding_box(s0, s1.p1)) { return dest; }
	}
	return NULL;
}
