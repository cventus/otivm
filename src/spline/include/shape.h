struct spline_segment
{
	float end[2], mid[2], weight;
};

struct spline_outline
{
	size_t n;
	struct spline_segment const *segments;
};

struct spline_shape
{
	size_t n;
	struct spline_outline const *outlines;
};

/* Create a simplified outline where no complex hull of three control poins
   overlap by splitting as necessary. This function does not handle
   intersecting curves otherwise. Free the returned shape with
   `spline_free_shape`. */
struct spline_shape *spline_simplify_shape(struct spline_shape const *shape);

void spline_free_shape(struct spline_shape *shape);
