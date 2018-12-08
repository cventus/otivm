#ifndef GENERIC_H_INCLUDED
#include "generic.h"
#endif

/* begin gm header */
#undef amapu
#define amapu MANGLE(amapu)
/* dest[i] = fn(arr[i]) */
T *amapu(T dest[], size_t n, T const a[], T (*fn)(T a));

#undef amaps
#define amaps MANGLE(amaps)
/* dest[i] = fn(arr[i], t) */
T *amaps(T dest[], size_t n, T const a[], T t, T (*fn)(T a, T t));

#undef amapb
#define amapb MANGLE(amapb)
/* dest[i] = fn(a[i], b[i]) */
T *amapb(T dest[], size_t n, T const a[], T const b[], T (*fn)(T a, T b));

#undef aset
#define aset MANGLE(aset)
/* Set each element, dest[i] = b  */
T *aset(T dest[], T b, size_t n);

#undef acopy
#define acopy MANGLE(acopy)
/* Set each element, dest[i] = a[i]  */
T *acopy(T dest[], T const a[], size_t n);

#undef aadds
#define aadds MANGLE(aadds)
/* Element-wise addition by scalar, dest[i] = a[i]+b  */
T *aadds(T dest[], size_t n, T const a[], T b);

#undef aadd
#define aadd MANGLE(aadd)
/* Element-wise array addition, dest[i] = a[i]+b[i] */
T *aadd(T dest[], size_t n, T const a[], T const b[]);

#undef asubs
#define asubs MANGLE(asubs)
/* Element-wise subtraction by scalar, dest[i] = a[i]-b  */
T *asubs(T dest[], size_t n, T const a[], T b);

#undef asub
#define asub MANGLE(asub)
/* Element-wise array subtraction, dest[i] = a[i]-b[i] */
T *asub(T dest[], size_t n, T const a[], T const b[]);

#undef amuls
#define amuls MANGLE(amuls)
/* Element-wise multiplication by scalar, dest[i] = a[i]*b */
T *amuls(T dest[], size_t n, T const a[], T b);

#undef amul
#define amul MANGLE(amul)
/* Element-wise array multiplication, dest[i] = a[i]*b[i] */
T *amul(T dest[], size_t n, T const a[], T const b[]);

#undef adivs
#define adivs MANGLE(adivs)
/* Element-wise division by scalar, dest[i] = a[i]/b */
T *adivs(T dest[], size_t n, T const a[], T b);

#undef adiv
#define adiv MANGLE(adiv)
/* Element-wise array division, dest[i] = a[i]/b[i] */
T *adiv(T dest[], size_t n, T const a[], T const b[]);

#undef asum
#define asum MANGLE(asum)
/* Sum of array elements */
T asum(T const a[], size_t n);

#undef aprod
#define aprod MANGLE(aprod)
/* Product of array elements */
T aprod(T const a[], size_t n);

#undef aneareqe
#define aneareqe MANGLE(aneareqe)
/* Nearly equal values with relative tolerance `reps` and absolute tolerance
   `aeps`. */
int aneareqe(T const a[], T const b[], size_t n, T reps, T aeps);

#undef aneareq
#define aneareq MANGLE(aneareq)
/* Nearly equal values with default tolerances. */
int aneareq(T const a[], T const b[], size_t n);

#undef amax
#define amax MANGLE(amax)
/* Get maximum value */
T amax(T const a[], size_t n);

#undef amin
#define amin MANGLE(amin)
/* Get minimum value */
T amin(T const a[], size_t n);
/* end gm header */
