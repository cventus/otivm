
struct gl_texture { GLuint name; };
struct gl_shader { GLuint name; };
struct gl_program { GLuint name; };

struct element_buffer
{
	GLuint name;
	GLsizei count;
	GLenum type, mode;
};

struct gl_geometry
{
	GLuint vao, vbo;
	struct element_buffer eb;
	struct gl_material const *material;
};

struct gl_geometries
{
	size_t n;
	struct gl_geometry const *geo;
};

