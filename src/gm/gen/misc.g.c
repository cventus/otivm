
#include "generic.h"
#include "misc.g.h"

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <float.h>

T frand(void)
{
	return (T)rand() / (T)RAND_MAX;
}

int fneareq(T a, T b)
{
	return fneareqe(a, b, LIT(1e-6), LIT(1e-8));
}

int fneareqe(T a, T b, T rel_epsilon, T abs_epsilon)
{
	T c = fminM(MAX, fmaxM(fabsM(a), fabsM(b)));
	return a == b || fabsM(a - b) <= (abs_epsilon + rel_epsilon*c);
}

T invsqrt(T val)
{
	assert(val > LIT(0.0));
	return LIT(1.0)/sqrtM(val);
}

