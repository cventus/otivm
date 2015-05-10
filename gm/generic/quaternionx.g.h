#define VECTOR_TAG q
#include "generic.h"
#include "vector.h"
#define Q(name) VECTOR_(q,name)

#define qcopy vcopy
#define qzero vzero
#define qneg vneg
#define qadd vadd
#define qsub vsub
#define qdot vdot
#define qlen2 vlen2
#define qlen vlen
#define qscale vscale
#define qnorm vnorm
#define qeqzeroe veqzeroe
#define qeqzero veqzero
#define qeqe veqe
#define qeq veq

/* Initialize with identity Identity */
#undef qid
#define qid Q(id)
T *qid(T dest[static 4]);

/* Create quaternion from axis and angle */
#undef qaxisangle
#define qaxisangle Q(axisangle)
T *qaxisangle(T dest[static 4], T angle, T const normal_axis[static 3]);

/* Spherical linear interpolation for time t, with a given angle omega */
#undef qslerpw
#define qslerpw Q(slerpw)
T *qslerpw(T dest[static 4], T const q0[static 4], T const q1[static 4],
             T omega, T t);

/* Spherical linear interpolation for time t */
#undef qslerp
#define qslerp Q(slerp)
T *qslerp(T dest[static 4], T const q0[static 4], T const q1[static 4], T t);

/* Quaternion multiplication */
#undef qmul
#define qmul Q(mul)
T *qmul(T dest[static 4], T const q0[static 4], T const q1[static 4]);

/* Quaternion conjugate */
#undef qconj
#define qconj Q(conj)
T *qconj(T dest[static 4], T const q[static 4]);

/* Rotate 3-vector u with a rotation defined by quaternion q */
#undef qrot
#define qrot Q(rot)
T *qrot(T dest[static 3], T const u[static 3], T const q[static 4]);

