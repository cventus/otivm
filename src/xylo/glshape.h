#include <stddef.h>

struct gl_api;
struct gl_core33;
struct xylo_glshape_set;
struct xylo_glshape;
struct xylo_shape;

struct xylo_glshape_set *xylo_make_glshape_set(
	struct gl_api *api,
	size_t n,
	struct xylo_shape const *shapes);

struct xylo_glshape const *xylo_get_glshape(
	struct xylo_glshape_set *set,
	size_t i);

void xylo_glshape_draw(
       struct gl_core33 const *restrict gl,
       struct xylo_glshape const *shape);

int xylo_free_glshape_set(struct xylo_glshape_set *set, struct gl_api *api);
