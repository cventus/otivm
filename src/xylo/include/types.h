
struct xylo;

struct xylo_leg
{
	float end[2], mid[2], weight;
};

struct xylo_outline
{
	struct xylo_leg *legs;
	size_t n;
};

struct xylo_shape
{
	size_t n;
	struct xylo_outline *outlines;
};

