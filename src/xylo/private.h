
#define GLSL(version,src) ("#version " #version " core\n" #src)

struct gl_api;
struct xylo;
struct xylo_shape;
struct xylo_glshape;
struct xylo_glshape_set;

struct spline_program;
struct points_program;
struct lines_program;
struct program_def;
struct program_uniform;

struct xylo *make_xylo(struct gl_api *);
void free_xylo(struct xylo *);
void xylo_begin(struct xylo *xylo);
void xylo_end(struct xylo *xylo);
void xylo_begin_points(struct xylo *xylo);
void xylo_end_points(struct xylo *xylo);
void xylo_begin_lines(struct xylo *xylo);
void xylo_end_lines(struct xylo *xylo);
void xylo_draw_shape(struct xylo *xylo, struct xylo_glshape *shape);
struct xylo_glshape_set *xylo_make_glshape_set(
	struct gl_api *gl,
	size_t n,
	struct xylo_shape const shapes[n]);

void xylo_set_shape_set(struct xylo *xylo, struct xylo_glshape_set *set);
int xylo_free_glshape_set(struct xylo_glshape_set *set, struct gl_api *gl);
struct xylo_glshape *xylo_get_glshape(struct xylo_glshape_set *set, size_t i);

GLuint xylo_make_program(struct gl_api *gl, struct program_def const *pdef);
void xylo_program_uniforms(
	struct gl_api *gl,
	GLuint program,
	void *dest,
	struct program_uniform const *uniforms);

int xylo_init_spline_program(struct spline_program *prog, struct gl_api *gl);
int xylo_term_spline_program(struct spline_program *prog, struct gl_api *gl);

void xylo_spline_set_to_clip(
	struct spline_program *prog,
	struct gl_api *gl,
	float const *matrix);

void xylo_spline_set_to_world(
	struct spline_program *prog,
	struct gl_api *gl,
	float const *matrix);

void xylo_spline_set_center(
	struct spline_program *prog,
	struct gl_api *gl,
	float x,
	float y);

void xylo_spline_set_frame_time(
	struct spline_program *prog,
	struct gl_api *gl,
	float t);

void xylo_spline_set_viewport(
	struct spline_program *prog,
	struct gl_api *gl,
	int x,
	int y,
	int w,
	int h);

int xylo_init_lines_program(struct lines_program *prog, struct gl_api *gl);
int xylo_term_lines_program(struct lines_program *prog, struct gl_api *gl);
void xylo_lines_set_to_clip(
	struct lines_program *prog,
	struct gl_api *gl,
	float const *matrix);
void xylo_lines_set_to_world(
	struct lines_program *prog,
	struct gl_api *gl,
	float const *matrix);
void xylo_lines_set_color(
	struct lines_program *prog,
	struct gl_api *gl,
	float red,
	float green,
	float blue);

int xylo_init_points_program(struct points_program *prog, struct gl_api *gl);
int xylo_term_points_program(struct points_program *prog, struct gl_api *gl);
void xylo_points_set_to_clip(
	struct points_program *prog,
	struct gl_api *gl,
	float const *matrix);
void xylo_points_set_to_world(
	struct points_program *prog,
	struct gl_api *gl,
	float const *matrix);
void xylo_points_set_ctrl(
	struct points_program *prog,
	struct gl_api *gl,
	float red,
	float green,
	float blue,
	float size);
void xylo_points_set_knot(
	struct points_program *prog,
	struct gl_api *gl,
	float red,
	float green,
	float blue,
	float size);

