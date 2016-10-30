
typedef int loadfn(void const *, size_t, void *, void *);

struct gl_vertex
{
	GLfloat position[3], normal[3], tcoord[2];
};

struct gl_material
{
	float ambient[3], diffuse[3], specular[3], exponent;
	GLuint program;
};

struct gl_programkey;

struct gl_vertexattrib
{
	GLuint index;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	GLvoid const *pointer;
};

struct gl_cache
{
	struct rescache *geometries;
	struct rescache *materials;
	struct rescache *textures;

	struct rescache *shaders;
	struct rescache *programs;

	struct rescache *wf_mtllibs;

	struct gl_material defmat;
	struct gl_api *gl;
};

