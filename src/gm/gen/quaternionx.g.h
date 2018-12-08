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

/* begin gm header */
#undef qid
#define qid Q(id)
/* Initialize with identity Identity */
T *qid(T dest[static 4]);

#undef qaxisangle
#define qaxisangle Q(axisangle)
/* Create quaternion from axis and angle */
T *qaxisangle(T dest[static 4], T angle, T const normal_axis[static 3]);

#undef qslerpw
#define qslerpw Q(slerpw)
/* Spherical linear interpolation for time t, with a given angle omega */
T *qslerpw(T dest[static 4], T const q0[static 4], T const q1[static 4],
             T omega, T t);

#undef qslerp
#define qslerp Q(slerp)
/* Spherical linear interpolation for time t */
T *qslerp(T dest[static 4], T const q0[static 4], T const q1[static 4], T t);

#undef qmul
#define qmul Q(mul)
/* Quaternion multiplication */
T *qmul(T dest[static 4], T const q0[static 4], T const q1[static 4]);

#undef qconj
#define qconj Q(conj)
/* Quaternion conjugate */
T *qconj(T dest[static 4], T const q[static 4]);

#undef qrot
#define qrot Q(rot)
/* Rotate 3-vector u with a rotation defined by quaternion q */
T *qrot(T dest[static 3], T const u[static 3], T const q[static 4]);
/* end gm header */
