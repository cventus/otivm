#ifndef GENERIC_H_INCLUDED
#include "generic.h"
#endif
#if N != M
#error "square-matrix.g.h functions are only defined for square matrices"
#endif
#include "matrix.g.h"

/* begin gm header */
#undef mdet
#define mdet MAT(det)
/* determinant */
T mdet(T const a[static M*N]);

#undef mtrace
#define mtrace MAT(trace)
/* trace */
T mtrace(T const a[static M*N]);

#undef minv
#define minv MAT(inv)
/* inverse */
T *minv(T a[static M*N], T const b[static M*N]);

#undef mpinv
#define mpinv MAT(pinv)
/* pseudo-inverse */
T *mpinv(T a[static M*N], T const b[static M*N]);
/* end gm header */
