#ifndef GENERIC_H
#include "generic.h"
#endif

#include <stddef.h>
#include <float.h>
#include <string.h>
#include "array.g.h"
#include "misc.g.h"

static T add(T a, T b) { return a + b; }
static T sub(T a, T b) { return a - b; }
static T mul(T a, T b) { return a * b; }
static T div(T a, T b) { return a / b; }

T *amaps(T dest[], size_t n, T const a[], T t, T (*fn)(T e, T t))
{
	size_t i;
	for (i = 0; i < n; i++) {
		dest[i] = fn(a[i], t);
	}
	return dest;
}

T *amapu(T dest[], size_t n, T const a[], T (*fn)(T a))
{
	size_t i;
	for (i = 0; i < n; i++) {
		dest[i] = fn(a[i]);
	}
	return dest;
}

T *amapb(T dest[], size_t n, T const a[], T const b[], T (*fn)(T a, T b))
{
	size_t i;
	for (i = 0; i < n; i++) {
		dest[i] = fn(a[i], b[i]);
	}
	return dest;
}

T *aset(T dest[], T b, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++) { dest[i] = b; }
	return dest;
}

T *acopy(T dest[], T const a[], size_t n)
{
	return memmove(dest,  a, n*sizeof(T));
}

T *aadds(T dest[], size_t n, T const a[], T b)
{
	return amaps(dest, n, a, b, add);
}

T *aadd(T dest[], size_t n, T const a[], T const b[])
{
	return amapb(dest, n, a, b, add);
}

T *asubs(T dest[], size_t n, T const a[], T b)
{
	return amaps(dest, n, a, b, sub);
}

T *asub(T dest[], size_t n, T const a[], T const b[])
{
	return amapb(dest, n, a, b, sub);
}

T *amuls(T dest[], size_t n, T const a[], T b)
{
	return amaps(dest, n, a, b, mul);
}

T *amul(T dest[], size_t n, T const a[], T const b[])
{
	return amapb(dest, n, a, b, mul);
}

T *adivs(T dest[], size_t n, T const a[], T b)
{
	return amaps(dest, n, a, b, div);
}

T *adiv(T dest[], size_t n, T const a[], T const b[])
{
	return amapb(dest, n, a, b, div);
}

T asum(T a[], size_t n)
{
	size_t i;
	T sum = LIT(0.0);
	for (i = 0; i < n; i++) {
		sum += a[i];
	}
	return sum;
}

T aprod(T a[], size_t n)
{
	size_t i;
	T prod = LIT(1.0);
	for (i = 0; i < n && prod != LIT(0.0); i++) {
		prod *= a[i];
	}
	return prod;
}

int aneareqe(T const a[], T const b[], size_t n, T reps, T aeps)
{
	size_t i;
	for (i = 0; i < n; i++) {
		if (!fneareqe(a[i], b[i], reps, aeps)) {
			return 0;
		}
	}
	return 1;
}

int aneareq(T const a[], T const b[], size_t n)
{
	return aneareqe(a, b, n, EQ_REL_EPSILON, EQ_ABS_EPSILON);
}

