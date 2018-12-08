#if defined(L) && L != 3
#error "Cross product only defined for vectors of length 3"
#endif
#ifndef VECTOR_G_H_INCLUDED
#include "vector.g.h"
#endif
#ifndef VECTOR_H_INCLUDED
#include "vector.h"
#endif

/* begin gm header */
#undef vcross
#define vcross V(cross)
/* Cross-product w = u x v */
T *vcross(T w[static L], T const u[static L], T const v[static L]);
/* end gm header */
