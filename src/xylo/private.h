struct gl_api;

struct xylo;
struct xylo_shapes;
struct xylo_quincunx;
struct xylo_glshape;
struct xylo_glshape_set;

#define GLSL(version, src) ("#version " #version " core\n" #src)

enum
{
	FILL_COLOR_LOC = 0,
	FRAGMENT_ID_LOC = 1
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

void xylo_shapes_set_object_id(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	unsigned object_id);

int xylo_init_quincunx(struct xylo_quincunx *quincunx, struct gl_api *api);

void xylo_term_quincunx(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl);

void xylo_quincunx_draw(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl);

void xylo_quincunx_set_pixel_size(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	float x_size, float y_size);

void xylo_quincunx_set_pixel_size2fv(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	float const *pixel_size);

void xylo_quincunx_set_center_unit(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	GLuint unit);

void xylo_quincunx_set_corner_unit(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	GLuint unit);
