#include <stddef.h>

struct gl_api;
struct gl_core33;
struct xylo_mesh_set;
struct xylo_mesh;
struct spline_shape;

struct xylo_mesh_set *xylo_make_mesh_set(
	struct gl_api *api,
	size_t n,
	struct spline_shape const *shapes);

void xylo_free_mesh_set(struct xylo_mesh_set *set, struct gl_api *api);

struct xylo_mesh const *xylo_get_mesh(
	struct xylo_mesh_set *set,
	size_t i);

void xylo_mesh_draw(
       struct gl_core33 const *restrict gl,
       struct xylo_mesh const *shape,
       GLsizei samples);
