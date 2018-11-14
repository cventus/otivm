#include "generic.h"

/* begin gm header */
#undef frand
#define frand MANGLE(frand)
/* Generage a random floating point number in the range [0, 1] (inclusive) */
T frand(void);

#undef fneareq
#define fneareq MANGLE(fneareq)
/* Test to see if a and b are almost equal using the default relative and
   absolute tolerances EQ_REL_EPSILON and EQ_ABS_EPSILON. */
int fneareq(T a, T b);

#undef fneareqe
#define fneareqe MANGLE(fneareqe)
/* Test to see if a and b are almost equal given relative and absolute
   tolerances. This function compares the difference between the `a` and `b`
   against the absolute and relative errors. The floating point difference is
   not commutative, so fneareqe(a, b, ...) is not neccesarily the same as
   fneareqe(b, a, ...). */
int fneareqe(T a, T b, T rel_epsilon, T abs_epsilon);

#undef invsqrt
#define invsqrt MANGLE(invsqrt)
/* Inverse square root */
T invsqrt(T val);
/* end gm header */
