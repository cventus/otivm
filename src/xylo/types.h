struct xylo_uniforms
{
	GLuint mvp, color;
};

struct xylo
{
	struct gl_api *api;
	GLuint program;
	struct xylo_uniforms uniforms;
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
