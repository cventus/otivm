#if defined(L) && L != 3
#error "Cross product only defined for vectors of length 3"
#endif
#undef VECTOR_TAG
#define L 3
#include "vector.h"
#include "vector.g.h"

/* Cross-product w = u x v */
#undef vcross
#define vcross V(cross)
T *vcross(T w[static L], T const u[static L], T const v[static L]);

