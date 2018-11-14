#ifndef GENERIC_H
#include "generic.h"
#endif
#define PLANE_G_H_INCLUDED

/* begin gm header */
#undef plane_normal
#define plane_normal MANGLE(plane_normal)
/* Find normal of the plane defined by three points `p0`, `p1`, and `p2`. If
   the points appear in counter-clockwise order, the normal points toward the
   observer. Store the result in `normal` and return it. */
T *plane_normal(T normal[static 3], const T p0[static 3], const T p1[static 3], const T p2[static 3]);

#undef defines_plane
#define defines_plane MANGLE(defines_plane)
/* Check whether three points can define a plane (i.e. they are distinct) */
int defines_plane(const T p0[static 3], const T p1[static 3], const T p2[static 3]);
/* end gm header */
