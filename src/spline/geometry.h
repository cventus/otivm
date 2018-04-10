#define EPSILON 1e-8
#define X 0
#define Y 1
#define XY 2

typedef float const float2[XY];

struct line2d { double n[XY], c; };

/* non-normalized signed distance to line */
double line2d_det(float2 a, float2 b, float2 c);

struct line2d make_line2d(float2 p0, float2 p1);

/* Signed distance from p to l. If the result is zero, then the point is on the
   line. If it is greater than zero then it is on the "right" side of l, where
   the orientation is determined when the line was created. A negative result
   indicates the point is to the "left". */
static inline double line2d_dist(struct line2d l, float2 p)
{
	/* offset dot product with line normal */
	return X[l.n]*X[p] + Y[l.n]*Y[p] + l.c;
}

/* check if point d is within circle specified by counter-clockwise points */
_Bool point2d_in_circle(float2 a, float2 b, float2 c, float2 d);

/* triangle defined by the positive side of three lines */
_Bool point2d_in_triangle(struct line2d const tri[3], float2 p);
