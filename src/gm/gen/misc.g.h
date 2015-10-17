
#include "generic.h"

/* Generage a random floating point number in the range [0, 1] (inclusive) */
#undef frand
#define frand MANGLE(frand)
T frand(void);

/* Test to see if a and b are almost equal using the default relative and
   absolute tolerances EQ_REL_EPSILON and EQ_ABS_EPSILON. */
#undef fneareq
#define fneareq MANGLE(fneareq)
int fneareq(T a, T b);

/* Test to see if a and b are almost equal given relative and absolute
   tolerances. This function compares the difference between the `a` and `b`
   against the absolute and relative errors. The floating point difference is
   not commutative, so fneareqe(a, b, ...) is not neccesarily the same as
   fneareqe(b, a, ...). */
#undef fneareqe
#define fneareqe MANGLE(fneareqe)
int fneareqe(T a, T b, T rel_epsilon, T abs_epsilon);

/* Inverse square root */
#undef invsqrt
#define invsqrt MANGLE(invsqrt)
T invsqrt(T val);

