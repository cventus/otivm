struct xylo
{
	struct gl_api *api;
	GLuint program, to_world, to_clip, center;
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
