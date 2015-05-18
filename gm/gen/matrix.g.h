#ifndef GENERIC_H
#include "generic.h"
#endif
#include "matrix.h"

/* Copy src matrix into dest */
#undef mcopy
#define mcopy MAT(copy)
T *mcopy(T dest[static M*N], T const src[static M*N]);

/* Create matrix with random elements (as random as rand()), each in the range
   [0 1] (inclusive)  */
#undef mrand
#define mrand MAT(rand)
T *mrand(T dest[static M*N]);

/* Zero every element */
#undef mzero
#define mzero MAT(zero)
T *mzero(T a[static M*N]);

/* Make every element one */
#undef mone
#define mone MAT(one)
T *mone(T a[static M*N]);

/* Make identity matrix */
#undef mid
#define mid MAT(id)
T *mid(T a[static M*N]);

/* Scalar times identity matrix */
#undef msid
#define msid MAT(sid)
T *msid(T a[static M*N], T s);

/* Transpose */
#undef mtranspose
#define mtranspose MAT(transpose)
T *mtranspose(T a[static M*N], T const b[static M*N]);

/* Matrix element-wise addition */
#undef madde
#define madde MAT(adde)
T *madde(T dest[M*N], T const a[static M*N], T const b[static M*N]);

/* Matrix element-wise addition by scalar */
#undef madds
#define madds MAT(adds)
T *madds(T dest[M*N], T const a[static M*N], T b);

/* Matrix element-wise subtraction */
#undef msube
#define msube MAT(sube)
T *msube(T dest[M*N], T const a[static M*N], T const b[static M*N]);

/* Matrix element-wise subtraction by scalar */
#undef msubs
#define msubs MAT(subs)
T *msubs(T dest[M*N], T const a[static M*N], T b);

/* Matrix element-wise multiplication */
#undef mmule
#define mmule MAT(mule)
T *mmule(T dest[M*N], T const a[static M*N], T const b[static M*N]);

/* Matrix element-wise multiplication by scalar */
#undef mmuls
#define mmuls MAT(muls)
T *mmuls(T dest[M*N], T const a[static M*N], T b);

/* Matrix element-wise division */
#undef mdive
#define mdive MAT(dive)
T *mdive(T dest[M*N], T const a[static M*N], T const b[static M*N]);

/* Matrix element-wise division by scalar */
#undef mdivs
#define mdivs MAT(divs)
T *mdivs(T dest[M*N], T const a[static M*N], T b);

/* Access element at row and column */
#undef mref
#define mref MAT(ref)
T mref(T const a[static M*N], unsigned row, unsigned column);

/* Print matrix to file (for debugging purposes mainly) */
#undef mfprint
#define mfprint MAT(fprint)
void mfprint(FILE *fp, T const a[static M*N]);

/* Print matrix to standard output (for debugging purposes mainly) */
#undef mprint
#define mprint MAT(print)
void mprint(T const a[static M*N]);

/* Fuzzy test for equality with given relative and absolute epsilon values */
#undef mneareqe
#define mneareqe MAT(neareqe)
int mneareqe(T const a[static M*N], T const b[static M*N], T rel_e, T abs_e);

/* Fuzzy test for equality with default epsilon value */
#undef mneareq
#define mneareq MAT(neareq)
int mneareq(T const a[static M*N], T const b[static M*N]);

