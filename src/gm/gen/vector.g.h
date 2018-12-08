#ifndef GENERIC_H
#include "generic.h"
#endif
#ifndef VECTOR_H_INCLUDED
#include "vector.h"
#endif
#define VECTOR_G_H_INCLUDED

/* begin gm header */
#undef vcopy
#define vcopy V(copy)
/* Copy vector: w = u, return w */
T *vcopy(T w[static L], const T u[static L]);

#undef vzero
#define vzero V(zero)
/* Zero-initialize vector: w = [0  0 ... 0], return w */
T *vzero(T w[static L]);

#undef vneg
#define vneg V(neg)
/* Negate: w = -u, return w */
T *vneg(T w[static L], const T u[static L]);

#undef vadd
#define vadd V(add)
/* Add: w = u + v, return w */
T *vadd(T w[static L], const T u[static L], const T v[static L]);

#undef vsub
#define vsub V(sub)
/* Subtract: w = u - v, return w */
T *vsub(T w[static L], const T u[static L], const T v[static L]);

#undef vdot
#define vdot V(dot)
/* Dot product: return u.v */
T vdot(const T u[static L], const T v[static L]);

#undef vlen2
#define vlen2 V(len2)
/* Squared vector length: return u.u */
T vlen2(const T u[static L]);

#undef vlen
#define vlen V(len)
/* Vector length: return sqrt(u.u) */
T vlen(const T u[static L]);

#undef vscale
#define vscale V(scale)
/* Multiply by scalar: w = [s*u_1  s*u_2 ... s*u_n], return w */
T *vscale(T w[static L], const T u[static L], T s);

#undef vnorm
#define vnorm V(norm)
/* Normalize: w = u*(1/|u|), return w */
T *vnorm(T w[static L], const T u[static L]);

#undef vtrynorm
#define vtrynorm V(trynorm)
/* Attempt to normalize vector u and store it in w, but check for null vectors.
   Return zero on success. */
int vtrynorm(T w[static L], const T u[static L]);

#undef vneareqe
#define vneareqe V(neareqe)
/* Fuzzy test for equality */
int vneareqe(const T u[static L], const T v[static L], T rel_e, T abs_e);

#undef vneareq
#define vneareq V(neareq)
int vneareq(const T u[static L], const T v[static L]);
/* end gm header */
