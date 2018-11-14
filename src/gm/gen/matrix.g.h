#ifndef GENERIC_H
#include "generic.h"
#endif
#include "matrix.h"
#define MREF(a, column, row) ((a)[(column) * N + (row)])

/* begin gm header */
#undef mcopy
#define mcopy MAT(copy)
/* Copy src matrix into dest */
T *mcopy(T dest[static M*N], T const src[static M*N]);

#undef mrand
#define mrand MAT(rand)
/* Create matrix with random elements (as random as rand()), each in the range
   [0 1] (inclusive)  */
T *mrand(T dest[static M*N]);

#undef mzero
#define mzero MAT(zero)
/* Zero every element */
T *mzero(T a[static M*N]);

#undef mone
#define mone MAT(one)
/* Make every element one */
T *mone(T a[static M*N]);

#undef mid
#define mid MAT(id)
/* Make identity matrix */
T *mid(T a[static M*N]);

#undef msid
#define msid MAT(sid)
/* Scalar times identity matrix */
T *msid(T a[static M*N], T s);

#undef mtranspose
#define mtranspose MAT(transpose)
/* Transpose */
T *mtranspose(T a[static M*N], T const b[static M*N]);

#undef madde
#define madde MAT(adde)
/* Matrix element-wise addition */
T *madde(T dest[M*N], T const a[static M*N], T const b[static M*N]);

#undef madds
#define madds MAT(adds)
/* Matrix element-wise addition by scalar */
T *madds(T dest[M*N], T const a[static M*N], T b);

#undef msube
#define msube MAT(sube)
/* Matrix element-wise subtraction */
T *msube(T dest[M*N], T const a[static M*N], T const b[static M*N]);

#undef msubs
#define msubs MAT(subs)
/* Matrix element-wise subtraction by scalar */
T *msubs(T dest[M*N], T const a[static M*N], T b);

#undef mmule
#define mmule MAT(mule)
/* Matrix element-wise multiplication */
T *mmule(T dest[M*N], T const a[static M*N], T const b[static M*N]);

#undef mmuls
#define mmuls MAT(muls)
/* Matrix element-wise multiplication by scalar */
T *mmuls(T dest[M*N], T const a[static M*N], T b);

#undef mdive
#define mdive MAT(dive)
/* Matrix element-wise division */
T *mdive(T dest[M*N], T const a[static M*N], T const b[static M*N]);

#undef mdivs
#define mdivs MAT(divs)
/* Matrix element-wise division by scalar */
T *mdivs(T dest[M*N], T const a[static M*N], T b);

#undef mref
#define mref MAT(ref)
/* Access element at row and column */
T mref(T const a[static M*N], unsigned row, unsigned column);

#undef mfprint
#define mfprint MAT(fprint)
/* Print matrix to file (for debugging purposes mainly) */
void mfprint(FILE *fp, T const a[static M*N]);

#undef mprint
#define mprint MAT(print)
/* Print matrix to standard output (for debugging purposes mainly) */
void mprint(T const a[static M*N]);

#undef mmax
#define mmax MAT(max)
/* Maximum element value */
T mmax(T const a[static M*N]);

#undef mmin
#define mmin MAT(min)
/* Minimum element value */
T mmin(T const a[static M*N]);

#undef mneareqe
#define mneareqe MAT(neareqe)
/* Fuzzy test for equality with given relative and absolute epsilon values */
int mneareqe(T const a[static M*N], T const b[static M*N], T rel_e, T abs_e);

#undef mneareq
#define mneareq MAT(neareq)
/* Fuzzy test for equality with default epsilon value */
int mneareq(T const a[static M*N], T const b[static M*N]);
/* end gm header */
