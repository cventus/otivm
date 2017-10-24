struct xylo_view
{
	float width, height;
};

struct xylo_leg
{
	float end[2], mid[2], weight;
};

struct xylo_outline
{
	size_t n;
	struct xylo_leg *legs;
};

struct xylo_shape
{
	size_t n;
	struct xylo_outline *outlines;
};
