#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdalign.h>

#include <gm/vector.h>
#include <gm/misc.h>
#include <base/mem.h>
#include <base/gbuf.h>
#include <base/wbuf.h>

#ifndef NAN
#error "NAN is not defined, use a C99 compatible compiler!"
#endif

#include "include/bspline.h"
#include "include/types.h"

/* number of floats from one spline element to the next */
static size_t stride(struct bspline2 const *b)
{
	return b->d * 2 + 1;
}

/* get the d-dimensional on-curve (Bézier end point) of a spline element */
static float *curve_point(struct bspline2 const *b, float *elem)
{
	return elem + b->d + 1;
}

/* get the d-dimensional basis splone control point of a spline element */
static float *control_point(struct bspline2 const *b, float *elem)
{
	(void)b;
	return elem + 1;
}

/* get the knot vector difference `u_{i+1} - u_i` */
static float *knot_span(struct bspline2 const *b, float *elem)
{
	(void)b;
	return elem;
}

int init_bspline2(struct bspline2 *b, size_t d)
{
	if (d < 1) { return -1; }
	b->d = d;
	init_gbuf(&b->spline);
	return 0;
}

void term_bspline2(struct bspline2 *b)
{
	term_gbuf(&b->spline);
}

static size_t ptsize(struct bspline2 const *b)
{
	return b->d * sizeof (float);
}

static size_t elem_size(struct bspline2 const *b)
{
	return stride(b) * sizeof (float);
}

/* get element with a wrapping index */
static float *get_elem(struct bspline2 *b, ptrdiff_t i)
{
	ptrdiff_t n, j;

	n = gbuf_size(&b->spline);
	assert(n > 0);
	j = (i * (ptrdiff_t)elem_size(b)) % n;
	return gbuf_get(&b->spline, (j < 0) ? j + n : j);
}

static void insert_at(struct bspline2 *b, size_t i)
{
	assert(i <= bspline2_size(b));
	gbuf_move_to(&b->spline, elem_size(b) * i);
}

static int reserve_points(struct bspline2 *b, size_t n)
{
	return gbuf_reserve(&b->spline, n*elem_size(b));
}

static float *alloc_point(struct bspline2 *b)
{
	return gbuf_alloc(&b->spline, elem_size(b));
}

static float *knot_point(struct bspline2 *b, ptrdiff_t k)
{
	float *left, *right, *lp, *cp, *rp;
	double la, ra, ls, rs;
	size_t i;

	assert(gbuf_size(&b->spline) > 0);

	left = get_elem(b, k-1);
	right = get_elem(b, k);

	cp = curve_point(b, right);
	if (isnan(*cp)) {
		/* zero? */
		ls = *knot_span(b, left);
		rs = *knot_span(b, right);

		la = ls / (ls + rs);
		ra = 1.0 - la;

		lp = control_point(b, left);
		rp = control_point(b, right);

		for (i = 0; i < b->d; i++) {
			cp[i] = la*lp[i] + ra*rp[i];
		}
	}
	return cp;
}

static ptrdiff_t get_span(struct bspline2 *b, double t, double *s)
{
	float *p;
	double x, y, c, len, r;
	size_t i, m;
	ptrdiff_t j;
	int retried;

	assert(b != NULL);
	assert(s != NULL);
	assert(isfinite(t));
	assert(gbuf_isuniform(&b->spline, elem_size(b), alignof(float)));

	m = stride(b);
	r = t;
	retried = 0;

	/* negative */
	if (r < 0.0) {
		len = bspline2_period(b);
		if (len <= 0.0) { return -1; }
		r = fmod(r, len);
		if (r < 0.0) {
			r += len;
		}
		assert(r >= 0.0);
		assert(r <= len);
	}

	/* subtract until t < span using Kahan summation */
again:	j = 0;
	len = c = 0.0;
	for (i = 0; i < 2; i++) {
		for (p = b->spline.begin[i]; p != b->spline.end[i]; p += m) {
			x = *knot_span(b, p) - c;
			y = len + x;
			if (r < y) {
				*s = r - len;
				return j;
			}
			c = (y - len) - x;
			len = y;
			j++;
		}
	}
	if (retried) { return -2; }
	if (len <= 0.0) { return -3; }
	if (!isfinite(len)) { return -4; }
	if (r >= len) {
		r = fmod(r, len);
		if (r >= len) {
			/* something bad is going on; avoid infinite loop */
			return -5;
		}
	}
	/* now r < len, try again */
	retried = 1;
	goto again;
}

/* Attempt to insert a new knot at curve position `t` which requires that two
   new control points are added and one removed. Return non-zero in case the
   new knot would result in a multiplicity greater than the the degree of the
   basis spline or if allocation fails. */
int bspline2_subdivide(struct bspline2 *b, double t)
{
	ptrdiff_t k;
	size_t i, j, elsz;
	double s, la, ra;
	float *left, *right, *cp, *lp, *in[3], *out[2];
	double num, denom;

	/* no range to insert into */
	if (bspline2_size(b) == 0) { return -1; }

	/* numerical error */
	k = get_span(b, t, &s);
	if (k < 0) { return -2; }

	/* fail early on out-of-memory */
	if (reserve_points(b, 1)) { return -3; }

	elsz = elem_size(b);

	in[0] = get_elem(b, k - 1);
	in[1] = get_elem(b, k);
	in[2] = get_elem(b, k + 1);

	out[0] = in[1];
	out[1] = (insert_at(b, k), alloc_point(b));

	/* initialize new element with contol point of the following */
	(void)memcpy(control_point(b, out[1]), control_point(b, in[2]), elsz);

	for (i = 2; i --> 0; ) {
		/* store new right-side knot and point */
		left = in[i];
		right = in[i + 1];

		for (num = s, j = 0; j < 1 - i; j++) {
			num += *knot_span(b, in[i + j]);
		}
		denom = *knot_span(b, left) + *knot_span(b, right);

		assert(denom > 0.0);

		/* coefficients for convex sum */
		la = num / denom;
		ra = 1.0 - la;

		lp = control_point(b, left);
		cp = control_point(b, out[j]);

		for (j = 0; j < b->d; j++) {
			cp[j] *= ra;
			cp[j] += la*lp[j];
		}
	}

	/* invalidate bezier points */
	*curve_point(b, out[1]) = NAN;
	for (i = 0; i < 3; i++) {
		*curve_point(b, in[i]) = NAN;
	}

	/* update knot span lengths */
	*knot_span(b, out[1]) = *knot_span(b, out[0]) - s;
	*knot_span(b, out[0]) = s;

	return 0;
}

static double *basis(double dst[3], double t)
{
	double s;

	assert (t >= 0.0);
	assert (t <= 1.0);
	
	s = 1.0 - t;
	dst[0] = s*s;
	dst[1] = 2*s*t;
	dst[2] = t*t;
	return dst;
}

static float *bez2(float *dst, double const b[3], float const *p[3], size_t d)
{
	size_t i;
	for (i = 0; i < d; i++) {
		dst[i] = b[0]*p[0][i] + b[1]*p[1][i] + b[2]*p[2][i];
	}
	return dst;
}

static float *rbez2(float *dst, double const b[3], float const *p[3], size_t d)
{
	size_t i;
	float buf;
	float const *w[3] = { p[0] + (d - 1), p[1] + (d - 1), p[2] + (d - 1) };
	double c[3] = { *w[0]*b[0], *w[1]*b[1], *w[2]*b[2] };
	(void)bez2(dst, c, p, d - 1);
	float denom = *bez2(&buf, b, w, 1);
	for (i = 0; i < d - 1; i++) { dst[i] /= denom; }
	return dst;
}

size_t bspline2_size(struct bspline2 const *b)
{
	return gbuf_nmemb(&b->spline, elem_size(b));
}

float bspline2_span(struct bspline2 *b, size_t i)
{
	if (i < bspline2_size(b)) {
		return *knot_span(b, get_elem(b, i));
	} else {
		return NAN;
	}
}

int bspline2_point(float *dest, struct bspline2 *b, size_t i)
{
	float const *p;
	if (i < bspline2_size(b)) {
		p = control_point(b, get_elem(b, i));
		(void)memcpy(dest, p, elem_size(b));
		return 0;
	} else {
		return -1;
	}
}

int bspline2_set_point(struct bspline2 *b, size_t i, float const *p)
{
	float *q;
	if (i < bspline2_size(b)) {
		q = get_elem(b, i);
		/* curve points become dirty */
		*curve_point(b, q) = NAN;
		*curve_point(b, get_elem(b, i-1)) = NAN;
		(void)memcpy(control_point(b, q), p, elem_size(b));
		return 0;
	} else {
		return -1;
	}
}

int bspline2_bezier_point(float *dest, struct bspline2 *b, size_t i)
{
	float const *p;
	if (i < bspline2_size(b)*2) {
		if (i & 1) {
			p = control_point(b, get_elem(b, i >> 1));
		} else {
			p = knot_point(b, i >> 1);
		}
		(void)memcpy(dest, p, elem_size(b));
		return 0;
	} else {
		return -1;
	}
}

double bspline2_period(struct bspline2 const *b)
{
	float *p;
	double x, y, c, len;
	size_t i, m;

	assert(b != NULL);
	assert(gbuf_isuniform(&b->spline, elem_size(b), alignof(float)));

	m = stride(b);
	len = c = 0.0;
	for (i = 0; i < 2; i++) {
		for (p = b->spline.begin[i]; p != b->spline.end[i]; p += m) {
			/* Kahan summation */
			x = *knot_span(b, p) - c;
			y = len + x;
			c = (y - len) - x;
			len = y;
		}
	}
	return len;
}

int bspline2_insert(struct bspline2 *b, size_t i, size_t n, float const *p)
{
	float const *q;
	float *el;
	size_t j;

	/* pre-allocate or fail early */
	if (reserve_points(b, n)) { return -1; }

	insert_at(b, i);
	for (j = 0, q = p; j < n; j++, q += b->d) {
		el = alloc_point(b);

		/* init knot span distance */
		el[0] = 1.f; /* TODO */

		/* copy control point */
		(void)memcpy(el + 1, q, ptsize(b));

		/* mark curve-point as dirty */
		el[b->d + 1] = NAN;
	}
	return 0;
}

/*
-------------------
s = 1, p = 2, h = 1
'''''''''''''''''''

P(k-p, 0) = P(k-2, 0) \
                        P(k-1, 1)
P(k-s, 0) = P(k-1, 0) /
                |
            P(k-0, 0)

for r in [1, 1] do
  for i in [k-1, k-1] do
      a(k-1, 1) = (u_k - u_{k-1}) / (u_{k+1} - u_{k-1})
      P'(k-1, 1) = (1 - a)P_{k-2} + a*P_{k-1}


(u_k - u_{k-1}) / (u_{k+1} - u_{k-1})

> w_k = u_{k+1} - u_k

> u_{k+1} - u_{k-1} = (u_{k+1} - u_k) + (u_k - u_{k-1})
                    = w_k + w_{k-1}


a(k-1, 1) = w_k / (w_k + w_{k-1})


1 - a(k-1, 1) = w_{k-1}/(w_k + w_{k-1})



        P'(k-1, 1) = (1 - a)P_{k-2} + a*P_{k-1}


    w_{k-1}/(w_k + w_{k-1})*P_{k-2}    +    w_k / (w_k + w_{k-1})*P_{k-1}


P'(k, 1) = w_{k}/(w_k + w_{k+1})*P_{k-1}    +    w_{k+1} / (w_k + w_{k+1})*P_{k}


*/

int bspline2_eval(float *result, struct bspline2 *b, double t)
{
	float const *p[3];
	double s, c[3];
	ptrdiff_t i;

	i = get_span(b, t, &s);
	if (i < 0) { return -1; }

	p[0] = knot_point(b, i);
	p[1] = control_point(b, get_elem(b, i));
	p[2] = knot_point(b, i+1);

	bez2(result, basis(c, s), p, b->d);
	return 0;
}

int bspline2_reval(float *result, struct bspline2 *b, double t)
{
	float const *p[3];
	double s, c[3];
	ptrdiff_t i;

	i = get_span(b, t, &s);
	if (i < 0) { return -1; }

	p[0] = knot_point(b, i);
	p[1] = control_point(b, get_elem(b, i));
	p[2] = knot_point(b, i+1);

	rbez2(result, basis(c, s), p, b->d);
	return 0;
}
