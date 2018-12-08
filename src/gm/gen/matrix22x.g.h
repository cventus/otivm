#ifndef GENERIC_H_INCLUDED
#include "generic.h"
#endif
#if !(N == 2) || !(M == 2)
#error "matrix22.g.h functions are only defined for 2x2 matrices"
#endif
#include "matrix.g.h"

/* begin gm header */
#undef mrot
#define mrot MAT(rot)
/* Initialize a rotation matrix */
T *mrot(T a[static M*N], T rad);

#undef mscale
#define mscale MAT(scale)
/* Initialize a scaling matrix */
T *mscale(T a[static M*N], T x, T y);

#undef muscale
#define muscale MAT(uscale)
/* Initialize a uniform scaling matrix */
T *muscale(T a[static M*N], T s);

#undef mmul
#define mmul MAT(mul)
/* Multiply two 2x2 matrices b and c and store in a and return it */
T *mmul(T a[restrict static M*N], T const b [static M*N], T const c [static M*N]);

#undef mshearx
#define mshearx MAT(shearx)
/* Transform vector u by a 2x2 matrix a and store in v and return it */
T *mshearx(T a[static M*N], T s);

#undef msheary
#define msheary MAT(sheary)
/* Transform vector u by a 2x2 matrix a and store in v and return it */
T *msheary(T a[static M*N], T s);
/* end gm header */
