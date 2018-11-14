#ifndef GENERIC_H_INCLUDED
#include "generic.h"
#endif
#if !(N == 4) || !(M == 4)
#error "matrix44.g.h functions are only defined for 4x4 matrices"
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

#undef mtranslate
#define mtranslate MAT(translate)
/* Initialize a translation matrix */
T *mtranslate(T a[static M*N], T x, T y, T z);

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
/* Multiply the two 4x4 matrices b and c and store in and return a */
T *mmul(T a[static M*N], T const b [restrict static M*N], T const c [restrict static M*N]);

#undef mmulv
#define mmulv MAT(mulv)
/* Transform 4x1 vector u with 4x4 matrix a (so that a*u = v) and store in v
   and return it */
T *mmulv(T v[static M*1], T const a [restrict static M*N], T const u [restrict static M*1]);

#undef mmulv3
#define mmulv3 MAT(mulv3)
/* Treat the 3x1 vectors u and v as 4x1 vectors where w = 1, with 4x4
   matrix a, the last row of which is assumed to be [0 0 0 1], and store in v
   and return it */
T *mmulv3(T v[static 3*1], T const a [restrict static M*N], T const u [restrict static 3*1]);

#undef mlookat
#define mlookat MAT(lookat)
/* Initialize a rotation matrix that looks at a certain point with some up
   direction */
T *mlookat(T result[static M*N], T const eye [static 3],
               T const target [static 3], T const up [static 3]);

#undef mperspective
#define mperspective MAT(perspective)
/* Initialize a perspective projection matrix */
T *mperspective(T a[static M*N], T fovy, T aspect, T znear, T zfar);

#undef morthographic
#define morthographic MAT(orthographic)
/* Initialize a orthographic projection matrix */
T *morthographic(T a[static M*N], T left, T right, T bottom, T top, T near, T far);

#undef mquat
#define mquat MAT(quat)
/* Initialize a rotation matrix from a quaternion */
T *mquat(T dest[static M*N], T const q[4]);
/* end gm header */
