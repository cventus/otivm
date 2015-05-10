#ifndef GENERIC_H
#include "generic.h"
#endif

/* dest[i] = fn(arr[i]) */
#undef amapu
#define amapu MANGLE(amapu)
T *amapu(T dest[], size_t n, T const a[], T (*fn)(T a));

/* dest[i] = fn(arr[i], t) */
#undef amaps
#define amaps MANGLE(amaps)
T *amaps(T dest[], size_t n, T const a[], T t, T (*fn)(T a, T t));

/* dest[i] = fn(a[i], b[i]) */
#undef amapb
#define amapb MANGLE(amapb)
T *amapb(T dest[], size_t n, T const a[], T const b[], T (*fn)(T a, T b));

/* Element-wise addition by scalar, dest[i] = a[i]+b  */
#undef aadds
#define aadds MANGLE(aadds)
T *aadds(T dest[], size_t n, T const a[], T b);

/* Element-wise array addition, dest[i] = a[i]+b[i] */
#undef aadd
#define aadd MANGLE(aadd)
T *aadd(T dest[], size_t n, T const a[], T const b[]);

/* Element-wise subtraction by scalar, dest[i] = a[i]-b  */
#undef asubs
#define asubs MANGLE(asubs)
T *asubs(T dest[], size_t n, T const a[], T b);

/* Element-wise array subtraction, dest[i] = a[i]-b[i] */
#undef asub
#define asub MANGLE(asub)
T *asub(T dest[], size_t n, T const a[], T const b[]);

/* Element-wise multiplication by scalar, dest[i] = a[i]*b */
#undef amuls
#define amuls MANGLE(amuls)
T *amuls(T dest[], size_t n, T const a[], T b);

/* Element-wise array multiplication, dest[i] = a[i]*b[i] */
#undef amul
#define amul MANGLE(amul)
T *amul(T dest[], size_t n, T const a[], T const b[]);

/* Element-wise division by scalar, dest[i] = a[i]/b */
#undef adivs
#define adivs MANGLE(adivs)
T *adivs(T dest[], size_t n, T const a[], T b);

/* Element-wise array division, dest[i] = a[i]/b[i] */
#undef adiv
#define adiv MANGLE(adiv)
T *adiv(T dest[], size_t n, T const a[], T const b[]);

/* Sum of array elements */
#undef asum
#define asum MANGLE(asum)
T asum(T a[], size_t n);

/* Product of array elements */
#undef aprod
#define aprod MANGLE(aprod)
T aprod(T a[], size_t n);

/* Nearly equal values with relative tolerance `reps` and absolute tolerance
   `aeps`. */
#undef aneareqe
#define aneareqe MANGLE(aneareqe)
int aneareqe(T const a[], T const b[], size_t n, T reps, T aeps);

/* Nearly equal values with default tolerances. */
#undef aneareq
#define aneareq MANGLE(aneareq)
int aneareq(T const a[], T const b[], size_t n);


