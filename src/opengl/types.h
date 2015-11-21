
typedef int loadfn(void const *, size_t, void *, void *);

struct gl_cache
{
	struct rescache *geometries;
	struct rescache *materials;
	struct rescache *textures;

	struct rescache *shaders;
	struct rescache *programs;

	struct rescache *wf_mtllibs;
};

struct gl_vertex
{
	GLfloat position[3], normal[3], tcoord[2];
};

struct gl_material
{
	float ambient[3], diffuse[3], specular[3], exponent;
	GLuint program;
};

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
	struct gl_geometry *geo;
};

struct gl_core
{
	/* Buffer Objects */
	void (*glGenBuffers)(GLsizei, GLuint *);
	void (*glDeleteBuffers)(GLsizei, GLuint const *);
	void (*glBindBuffer)(GLenum, GLuint);
	void (*glBufferData)(GLenum, GLsizeiptr, GLvoid const *, GLenum);
	GLboolean (*glIsBuffer)(GLuint);

	/* Shaders */
	GLuint (*glCreateShader)(GLenum);
	void (*glDeleteShader)(GLuint);
	GLboolean (*glIsShader)(GLuint);
	void (*glShaderSource)(GLuint, GLsizei, GLchar const **, GLint const *);
	void (*glGetShaderSource)(GLuint, GLsizei, GLsizei *, GLchar *);
	void (*glCompileShader)(GLuint);
	void (*glGetShaderiv)(GLuint, GLenum, GLint *);
	void (*glGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);

	/* Programs */
	GLuint (*glCreateProgram)(void);
	void (*glDeleteProgram)(GLuint);
	void (*glAttachShader)(GLuint, GLuint);
	void (*glDetachShader)(GLuint, GLuint);
	void (*glGetProgramiv)(GLuint, GLenum, GLint *);
	void (*glGetProgramInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);
	GLint (*glGetFragDataLocation)(GLuint, GLchar const *);
	void (*glBindFragDataLocation)(GLuint, GLuint, GLchar const *);
	void (*glLinkProgram)(GLuint);
	void (*glUseProgram)(GLuint);
	GLint (*glGetUniformLocation)(GLuint, GLchar const *);
	void (*glUniform3f)(GLint, GLfloat, GLfloat, GLfloat);
	GLint (*glGetAttribLocation)(GLuint, GLchar const *);
	void (*glBindAttribLocation)(GLuint, GLuint, GLchar const *);
	void (*glEnableVertexAttribArray)(GLuint);
	void (*glDisableVertexAttribArray)(GLuint);
	GLint (*glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean,
	                               GLsizei, GLvoid const *);

	/* Vertex Array Objects */
	void (*glGenVertexArrays)(GLsizei, GLuint *);
	void (*glBindVertexArray)(GLuint);
	void (*glDeleteVertexArrays)(GLsizei, GLuint const *);
	GLboolean (*glIsVertexArray)(GLuint);

	/* Other */
	const GLubyte *(*glGetStringi)(GLenum, GLuint);
};

struct gl_texture { GLuint name; };
struct gl_shader { GLuint name; };

struct gl_program
{
	struct gl_shader *vertex, *fragment;
	GLuint name;
};

struct gl_programkey;

struct gl_state
{
	struct gl_core f;
	struct gl_cache cache;
	struct gl_material defmat;
};

struct gl_vertexattrib
{
	GLuint index;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	GLvoid const *pointer;
};

