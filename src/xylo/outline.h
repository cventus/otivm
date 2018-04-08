#include <stddef.h>

struct gl_api;
struct gl_core33;
struct xylo_outline_set;
struct xylo_outline;
struct spline_shape;

struct xylo_outline_set *xylo_make_outline_set(
	struct gl_api *api,
	size_t n,
	struct spline_shape const *shapes);

void xylo_free_outline_set(struct xylo_outline_set *set, struct gl_api *api);

struct xylo_outline const *xylo_get_outline(
	struct xylo_outline_set *set,
	size_t i);

void xylo_outline_draw(
       struct gl_core33 const *restrict gl,
       struct xylo_outline const *shape,
       GLsizei samples);
