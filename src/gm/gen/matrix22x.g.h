#ifndef GENERIC_H_INCLUDED
#include "generic.h"
#endif
#if !(N == 2) || !(M == 2)
#error "matrix22.g.h functions are only defined for 2x2 matrices"
#endif
#include "matrix.g.h"

/* Initialize a rotation matrix */
#undef mrot
#define mrot MAT(rot)
T *mrot(T a[static M*N], T rad);

/* Initialize a scaling matrix */
#undef mscale
#define mscale MAT(scale)
T *mscale(T a[static M*N], T x, T y);

/* Initialize a uniform scaling matrix */
#undef muscale
#define muscale MAT(uscale)
T *muscale(T a[static M*N], T s);

/* Multiply two 2x2 matrices b and c and store in a and return it */
#undef mmul
#define mmul MAT(mul)
T *mmul(T a[restrict static M*N], T const b [static M*N], T const c [static M*N]);

/* Transform vector u by a 2x2 matrix a and store in v and return it */
#undef mshearx
#define mshearx MAT(shearx)
T *mshearx(T a[static M*N], T s);

/* Transform vector u by a 2x2 matrix a and store in v and return it */
#undef msheary
#define msheary MAT(sheary)
T *msheary(T a[static M*N], T s);
