
struct shader
{
	GLchar const **src;
	GLenum type;
};

struct program_location
{
	char const *name;
	GLuint loc;
};

struct program_def
{
	struct shader const *shaders;
	struct program_location const *attrib;
	struct program_location const *frag;
};

struct program_uniform
{
	char const *name;
	size_t offset;
};

struct spline_program
{
	GLuint name;

	/* uniforms */
	GLuint to_world, to_clip, center, frame_time, viewport;
};

struct points_program
{
	GLuint name;

	/* uniforms */
	GLuint to_world, to_clip, ctrl_color_size, knot_color_size;
};

struct lines_program
{
	GLuint name;

	/* uniforms */
	GLuint to_world, to_clip, color;
};

struct xylo
{
	struct gl_api *gl;

	/* programs */
	struct spline_program spline;
	struct points_program points;
	struct lines_program lines;
};

struct xylo_glshape
{
	GLint *first;
	GLsizei *count, drawcount;
};

struct xylo_glshape_set
{
	GLuint vao, vbo;
	struct xylo_glshape *shapes;
	size_t n;
};

struct xylo_fb
{
	GLuint fbo, samptex;
	unsigned width, height;
};

