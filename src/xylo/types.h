struct xylo_shapes
{
	GLuint program;
	GLuint mvp, color, object_id;
};

struct xylo_quincunx
{
	GLuint program, vao, vbo;
	GLuint pixel_size, center_tex, corner_tex;
};

struct saved_state
{
	_Bool stencil_test: 1;
	_Bool multisample: 1;
};

struct xylo_fb
{
	GLuint fbo, color, object_id, ds;
	GLsizei width, height;
};

struct xylo
{
	struct gl_api *api;
	struct xylo_shapes shapes;
	struct xylo_quincunx quincunx;
	struct xylo_fb center_samples, corner_samples;
	unsigned begin;
	struct saved_state save;
};

struct xylo_glshape
{
	void const *indices;
	GLsizei count;
	GLenum type;
	GLint basevertex;
};

struct xylo_glshape_set
{
	size_t n;
	struct xylo_glshape *shapes;
	GLuint vao, evbo;
	GLsizei vertex_offset;
};
