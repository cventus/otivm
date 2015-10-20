
#include <GL/gl.h>
#include <GL/glext.h>

#include "types.h"
#include "decl.h"

int gl_resolve_functions(struct glfn *glfn)
{
#define resolve(res,name,args) do { \
		 glfn->name = (res(*)args)gl_get_proc(#name); \
		if (!glfn->name) { return -1; } \
	} while (0)


	/* Buffer Objects */
	resolve(void,glGenBuffers,(GLsizei, GLuint *));
	resolve(void,glDeleteBuffers,(GLsizei, GLuint const *));
	resolve(void,glBindBuffer,(GLenum, GLuint));
	resolve(void,glBufferData,(GLenum, GLsizeiptr, GLvoid const *, GLenum));
	resolve(GLboolean,glIsBuffer,(GLuint));

	/* Shaders */
	resolve(GLuint,glCreateShader,(GLenum));
	resolve(void,glDeleteShader,(GLuint));
	resolve(GLboolean,glIsShader,(GLuint));
	resolve(void,glShaderSource,(GLuint, GLsizei, GLchar const **, GLint const *));
	resolve(void,glGetShaderSource,(GLuint, GLsizei, GLsizei *, GLchar *));
	resolve(void,glCompileShader,(GLuint));
	resolve(void,glGetShaderiv,(GLuint, GLenum, GLint *));
	resolve(void,glGetShaderInfoLog,(GLuint, GLsizei, GLsizei *, GLchar *));

	/* Programs */
	resolve(GLuint,glCreateProgram,(void));
	resolve(void,glDeleteProgram,(GLuint));
	resolve(void,glAttachShader,(GLuint, GLuint));
	resolve(void,glDetachShader,(GLuint, GLuint));
	resolve(void,glGetProgramiv,(GLuint, GLenum, GLint *));
	resolve(void,glGetProgramInfoLog,(GLuint, GLsizei, GLsizei *, GLchar *));
	resolve(GLint,glGetFragDataLocation,(GLuint, GLchar const *));
	resolve(void,glBindFragDataLocation,(GLuint, GLuint, GLchar const *));
	resolve(void,glLinkProgram,(GLuint));
	resolve(void,glUseProgram,(GLuint));
	resolve(GLint,glGetUniformLocation,(GLuint, GLchar const *));
	resolve(void,glUniform3f,(GLint, GLfloat, GLfloat, GLfloat));
	resolve(GLint,glGetAttribLocation,(GLuint, GLchar const *));
	resolve(void,glBindAttribLocation,(GLuint, GLuint, GLchar const *));
	resolve(void,glEnableVertexAttribArray,(GLuint));
	resolve(void,glDisableVertexAttribArray,(GLuint));
	resolve(GLint,glVertexAttribPointer,(GLuint, GLint, GLenum, GLboolean,
	                                     GLsizei, GLvoid const *));

	/* Vertex Array Objects */
	resolve(void,glGenVertexArrays,(GLsizei, GLuint *));
	resolve(void,glBindVertexArray,(GLuint));
	resolve(void,glDeleteVertexArrays,(GLsizei, GLuint const *));
	resolve(GLboolean,glIsVertexArray,(GLuint));
#undef resolve

	return 0;
}

