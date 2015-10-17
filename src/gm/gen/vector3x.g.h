#if defined(L) && L != 3
#error "Cross product only defined for vectors of length 3"
#endif
#ifndef VECTOR_G_H_INCLUDED
#include "vector.g.h"
#endif
#ifndef VECTOR_H_INCLUDED
#include "vector.h"
#endif

/* Cross-product w = u x v */
#undef vcross
#define vcross V(cross)
T *vcross(T w[static L], T const u[static L], T const v[static L]);

