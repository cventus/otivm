#ifndef GENERIC_H_INCLUDED
#include "generic.h"
#endif
#if N != M
#error "square-matrix.g.h functions are only defined for square matrices"
#endif
#include "matrix.g.h"

/* determinant */
#undef mdet
#define mdet MAT(det)
T mdet(T const a[static M*N]);

/* trace */
#undef mtrace
#define mtrace MAT(trace)
T mtrace(T const a[static M*N]);

/* inverse */
#undef minv
#define minv MAT(inv)
T *minv(T a[static M*N], T const b[static M*N]);

/* pseudo-inverse */
#undef mpinv
#define mpinv MAT(pinv)
T *mpinv(T a[static M*N], T const b[static M*N]);

