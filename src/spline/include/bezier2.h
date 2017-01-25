/* Evaluate the quadratic bézier curve at t in [0, 1] which is defined by the
   three consecutive d-dimensional control points that p points to and store it
   in the d-dimensional point dest. Return dest. */
float *bezier2(float *dest, size_t d, float const *p, double t);

/* Evaluate the rational quadratic bézier curve at t in [0, 1] which is defined
   by the three consecutive (d+1)-dimensional control points that p points to
   (i.e. total size is (d+1) * 3), where the final extra component is the
   weight, and store it in the d-dimensional point dest. Return dest. */
float *rbezier2(float *dest, size_t d, float const *p, double t);

/* A quadratic rational Bézier spline with control points { p0, p1, p2 } and
   corresponding weights { w0, w1, w2 } is equivalent to one with weights
   { 1, w', 1 } where w'=(w1*w1)/(w0*w2), which reduces the number of
   parameters by two.

   rbezier2_norm_w1(w0, w1, w2) returns the normalized weight w' */
float rbezier2_norm_w1(float w0, float w1, float w2);
