#ifndef GENERIC_H_INCLUDED
#include "generic.h"
#endif
#if !(N == 3) || !(M == 3)
#error "matrix33.g.h functions are only defined for 3x3 matrices"
#endif
#include "matrix.g.h"

/* Initialize a rotation matrix (x-axis) */
#undef mrotx
#define mrotx MAT(rotx)
T *mrotx(T a[static M*N], T rad);

/* Initialize a rotation matrix (y-axis) */
#undef mroty
#define mroty MAT(roty)
T *mroty(T a[static M*N], T rad);

/* Initialize a rotation matrix (z-axis) */
#undef mrotz
#define mrotz MAT(rotz)
T *mrotz(T a[static M*N], T rad);

/* Initialize a scaling matrix */
#undef mscale
#define mscale MAT(scale)
T *mscale(T a[static M*N], T x, T y, T z);

/* Initialize a uniform scaling matrix */
#undef muscale
#define muscale MAT(uscale)
T *muscale(T a[static M*N], T s);

/* Multiply two 3x3 matrices b and c and store in v and return it */
#undef mmul
#define mmul MAT(mul)
T *mmul(T a[static M*N], T const b [static M*N], T const c [static M*N]);

/* Rotate vector u with 3x3 matrix a and store in v and return it */
#undef mmulv
#define mmulv MAT(mulv)
T *mmulv(T v[static N], T const b [static M*N], T const u [static N]);

/* Initialize a rotation matrix from a quaternion */
#undef mquat
#define mquat MAT(quat)
T *mquat(T dest[static M*N], T const q[4]);

