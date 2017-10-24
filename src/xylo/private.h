struct gl_api;
struct xylo;
struct xylo_shapes;
struct xylo_glshape;
struct xylo_glshape_set;

#define GLSL(version,src) ("#version " #version " core\n" #src)

enum
{
	FILL_COLOR_LOC = 0
};

enum
{
	ATTRIB_SHAPE_POS = 0,
	ATTRIB_QUADRATIC_POS = 1
};

GLuint xylo_get_uint(struct gl_core33 const *restrict gl, GLenum t);

void xylo_draw_glshape(struct xylo *xylo, struct xylo_glshape const *shape);

void xylo_set_shape_set(struct xylo *xylo, struct xylo_glshape_set *set);
struct xylo_glshape const *xylo_get_glshape(
	struct xylo_glshape_set *set,
	size_t i);

int xylo_init_shapes(struct xylo_shapes *shapes, struct gl_api *api);

void xylo_shapes_set_color4fv(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	float const *color);

void xylo_shapes_set_mvp(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	float const *mvp);
