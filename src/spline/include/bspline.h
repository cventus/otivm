struct bspline2;

/* initiate a d-dimensional quadratic basis spline (d > 0) */
int init_bspline2(struct bspline2 *, size_t d);

int copy_bspline2(struct bspline2 *dest, struct bspline2 const *src);
struct bspline2 *clone_bspline2(struct bspline2 const *src);

void term_bspline2(struct bspline2 *);

/* Return the number of control points in the basis spline. */
size_t bspline2_size(struct bspline2 const *b);

/* The period length of the spline for evaluation. */
double bspline2_period(struct bspline2 const *b);

/* Add new control points at index 0 <= i <= bspline2_size. */
int bspline2_insert(struct bspline2 *b, size_t i, size_t n, float const *p);

/* Split the knot span at t without changing the shape of the curve by creating
   new points and removing old ones as necessary. */
int bspline2_subdivide(struct bspline2 *b, double t);

/* Copy the kth control point into dest where 0 <= k < bspline2_size. Return
   non-zero if k is outside of this range. */
int bspline2_point(float *dest, struct bspline2 *b, size_t k);

/* Set the kth control point to pos where 0 <= k < bspline2_size. Return
   non-zero if k is outside of this range. */
int bspline2_set_point(struct bspline2 *b, size_t k, float const *pos);

/* A basis spline can be converted into a sequence of Bézier splines where the
   first and and last Bézier control point of each spline segment are shared
   among the segments. Return the number of bezier points in the basis spline.
   */
size_t bspline2_bezier_size(struct bspline2 const *b);

/* Copy the kth Bézier control point into dest where 0 <= k < 2*bspline2_size.
   Points with even index are (shared) end-points of Bézier curves. Points with
   odd index are internal control points of Bézier splines. E.g. the points
   { 0, 1, 2 } forms one bezier spline and { 2, 3, 4 } the next, and so on. The
   Bézier points and curve can only be manipulated through the basis spline
   control points. */
int bspline2_bezier_point(float *dest, struct bspline2 *b, size_t k);

/* The length of the spline. */
double bspline2_length(struct bspline2 const *b);

/* The length of the spline when interpreted as a rational b-spline
   (the final component of each point is the weight). */
double bspline2_rlength(struct bspline2 const *b);

/* Get the point on the curve at t. */
int bspline2_eval(float *dest, struct bspline2 *b, double t);

/* Get the point on the curve at t  when interpreted as a rational b-spline
   (where the final component as the point weight). */
int bspline2_reval(float *dest, struct bspline2 *b, double t);

/* Get the offset t on the curve which is closes to the point p. */
double bspline2_nearest(struct bspline2 const *b, float const *p);

/* Get the offset t on the curve when interpreted as a rational b-spline
   (where the final point component is the weight) that is closest to
   point p. */
double bspline2_rnearest(struct bspline2 const *b, float const *p);
