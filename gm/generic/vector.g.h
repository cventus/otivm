#ifndef GENERIC_H
#include "generic.h"
#endif
#include "vector.h"

/* Copy vector: w = u, return w */
#undef vcopy
#define vcopy V(copy)
T *vcopy(T w[static L], const T u[static L]);

/* Zero-initialize vector: w = [0  0 ... 0], return w */
#undef vzero
#define vzero V(zero)
T *vzero(T w[static L]);

/* Negate: w = -u, return w */
#undef vneg
#define vneg V(neg)
T *vneg(T w[static L], const T u[static L]);

/* Add: w = u + v, return w */
#undef vadd
#define vadd V(add)
T *vadd(T w[static L], const T u[static L], const T v[static L]);

/* Subtract: w = u - v, return w */
#undef vsub
#define vsub V(sub)
T *vsub(T w[static L], const T u[static L], const T v[static L]);

/* Dot product: return u.v */
#undef vdot
#define vdot V(dot)
T vdot(const T u[static L], const T v[static L]);

/* Squared vector length: return u.u */
#undef vlen2
#define vlen2 V(len2)
T vlen2(const T u[static L]);

/* Vector length: return sqrt(u.u) */
#undef vlen
#define vlen V(len)
T vlen(const T u[static L]);

/* Multiply by scalar: w = [s*u_1  s*u_2 ... s*u_n], return w */
#undef vscale
#define vscale V(scale)
T *vscale(T w[static L], const T u[static L], T s);

/* Normalize: w = u*(1/|u|), return w */
#undef vnorm
#define vnorm V(norm)
T *vnorm(T w[static L], const T u[static L]);

/* Attempt to normalize vector u and store it in w, but check for null vectors.
   Return zero on success. */
#undef vtrynorm
#define vtrynorm V(trynorm)
int vtrynorm(T w[static L], const T u[static L]);

/* Fuzzy test for equality */
#undef vneareqe
#define vneareqe V(neareqe)
int vneareqe(const T u[static L], const T v[static L], T rel_e, T abs_e);

#undef vneareq
#define vneareq V(neareq)
int vneareq(const T u[static L], const T v[static L]);

