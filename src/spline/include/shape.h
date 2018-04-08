struct spline_segment
{
	float end[2], mid[2], weight;
};

struct spline_outline
{
	size_t n;
	struct spline_segment *segments;
};

struct spline_shape
{
	size_t n;
	struct spline_outline *outlines;
};
