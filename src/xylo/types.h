struct xylo_shapes
{
	GLuint program;
	GLuint mvp, sample_clip, sample_offset, color, object_id;
};

struct xylo_quincunx
{
	GLuint program, vao, vbo;
	GLuint pixel_size, tex;
};

struct xylo_rgss
{
	GLuint program, vao, vbo;
	GLuint tex;
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
	struct xylo_rgss rgss;
	struct xylo_fb samples;
	unsigned begin;
	struct saved_state save;
	int aa;
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
