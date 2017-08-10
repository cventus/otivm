#ifndef GENERIC_H_INCLUDED
#include "generic.h"
#endif
#if !(N == 4) || !(M == 4)
#error "matrix44.g.h functions are only defined for 4x4 matrices"
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

/* Initialize a translation matrix */
#undef mtranslate
#define mtranslate MAT(translate)
T *mtranslate(T a[static M*N], T x, T y, T z);

/* Initialize a scaling matrix */
#undef mscale
#define mscale MAT(scale)
T *mscale(T a[static M*N], T x, T y, T z);

/* Initialize a uniform scaling matrix */
#undef muscale
#define muscale MAT(uscale)
T *muscale(T a[static M*N], T s);

/* Multiply the two 4x4 matrices b and c and store in and return a */
#undef mmul
#define mmul MAT(mul)
T *mmul(T a[static M*N], T const b [static M*N], T const c [static M*N]);

/* Rotate vector u with 4x4 matrix a and store in v and return it */
#undef mmulv
#define mmulv MAT(mulv)
T *mmulv(T v[static N], T const b [static M*N], T const u [static N]);

/* Initialize a rotation matrix that looks at a certain point with some up
   direction */
#undef mlookat
#define mlookat MAT(lookat)
T *mlookat(T result[static M*N], T const eye [static 3],
               T const target [static 3], T const up [static 3]);

/* Initialize a perspective projection matrix */
#undef mperspective
#define mperspective MAT(perspective)
T *mperspective(T a[static M*N], T fovy, T aspect, T znear, T zfar);

/* Initialize a orthographic projection matrix */
#undef morthographic
#define morthographic MAT(orthographic)
T *morthographic(T a[static M*N], T left, T right, T bottom, T top, T near, T far);

/* Initialize a rotation matrix from a quaternion */
#undef mquat
#define mquat MAT(quat)
T *mquat(T dest[static M*N], T const q[4]);
