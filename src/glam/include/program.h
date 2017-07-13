/* name of a uniform and offset where to store it a container structure */
struct gl_uniform_layout
{
	ptrdiff_t offset;
	GLchar const *name;
};

/* shader source and type */
struct gl_shader_source
{
	GLenum type;
	GLchar const *source;
};

/* name and index of an attribute or fragment date location */
struct gl_location
{
	GLuint index;
	GLchar const *name;
};

/* program description including arrays of shaders and attribute and
   fragment data locations, each of which should be terminated with a
   value of `{ 0, 0 }`. */
struct gl_program_layout
{
	struct gl_shader_source const *source;
	struct gl_location const *attrib, *frag;
};

struct gl_api;

/* create a shader program based on a program layout structure */
GLuint gl_make_program(struct gl_api *gl, struct gl_program_layout const *);

/* if `name` is the current program in use, make program `0` the current
   program, thereby freeing it */
void gl_unuse_program(struct gl_api *gl, GLuint name);

/* store the locations of the uniform declared in the `uo` array (terminated by
   a `{ 0, 0 }` value) found in the program into the object pointed to by
   `dest` */
void gl_get_uniforms(
        struct gl_api *gl,
        void *dest,
        GLuint program,
        struct gl_uniform_layout const *uo);
