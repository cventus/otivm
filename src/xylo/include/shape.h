struct spline_shape;

struct xylo_outline;
struct xylo_outline_set;

struct gl_api;

struct xylo_outline_set *xylo_make_outline_set(
	struct gl_api *gl,
	size_t n,
	struct spline_shape const *shapes);

struct xylo_outline const *xylo_get_outline(
	struct xylo_outline_set *set,
	size_t i);

void xylo_free_outline_set(
	struct xylo_outline_set *set,
	struct gl_api *gl);
