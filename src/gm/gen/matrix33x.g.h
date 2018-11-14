#ifndef GENERIC_H_INCLUDED
#include "generic.h"
#endif
#if !(N == 3) || !(M == 3)
#error "matrix33.g.h functions are only defined for 3x3 matrices"
#endif
#include "matrix.g.h"

/* begin gm header */
#undef mrotx
#define mrotx MAT(rotx)
/* Initialize a rotation matrix (x-axis) */
T *mrotx(T a[static M*N], T rad);

#undef mroty
#define mroty MAT(roty)
/* Initialize a rotation matrix (y-axis) */
T *mroty(T a[static M*N], T rad);

#undef mrotz
#define mrotz MAT(rotz)
/* Initialize a rotation matrix (z-axis) */
T *mrotz(T a[static M*N], T rad);

#undef mscale
#define mscale MAT(scale)
/* Initialize a scaling matrix */
T *mscale(T a[static M*N], T x, T y, T z);

#undef muscale
#define muscale MAT(uscale)
/* Initialize a uniform scaling matrix */
T *muscale(T a[static M*N], T s);

#undef mmul
#define mmul MAT(mul)
/* Multiply two 3x3 matrices b and c and store in v and return it */
T *mmul(T a[static M*N], T const b [static M*N], T const c [static M*N]);

#undef mmulv
#define mmulv MAT(mulv)
/* Rotate vector u with 3x3 matrix a and store in v and return it */
T *mmulv(T v[static N], T const b [static M*N], T const u [static N]);

#undef mquat
#define mquat MAT(quat)
/* Initialize a rotation matrix from a quaternion */
T *mquat(T dest[static M*N], T const q[4]);
/* end gm header */
