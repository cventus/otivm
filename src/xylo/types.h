struct xylo_shapes
{
	GLuint program;
	GLuint mvp, color;
};

struct saved_state
{
	_Bool stencil_test: 1;
	_Bool multisample: 1;
};

struct xylo
{
	struct gl_api *api;
	struct xylo_shapes shapes;
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
