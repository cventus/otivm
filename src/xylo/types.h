struct xylo_uniforms
{
	GLuint mvp, color, center;
};

struct xylo
{
	struct gl_api *api;
	GLuint program;
	struct xylo_uniforms uniforms;
};

struct xylo_glshape
{
	GLint *first;
	GLsizei *count, drawcount;
};

struct xylo_glshape_set
{
	struct xylo_glshape *shapes;
	GLuint vao, vbo;
	size_t n;
};
