/* Quadratic periodic basis spline
 *
 * A basis spline is defined by a knot-vector [u_0, ..., u_m], u_i <= u_{i+1},
 * control points [p_0, ..., p_m], and a degree p > 0, where typically
 * m >= n + p - 1.  The knot vector specifies the length of the curve and also
 * divides the length of the curve into segments (u_{i+1} - u_i). A periodic or
 * closed curve forms a loop where points and segment lengths repeat. Basis
 * splines can be turned into Bézier splines where the end-points correspond to
 * knots, and in particular in closed curves every segment correspond to one
 * Bézier curve.
 */
struct bspline2
{
	/* dimension */
	size_t d;

	/* each spline element consists of 1+2d floats */
	struct gbuf spline;
};
