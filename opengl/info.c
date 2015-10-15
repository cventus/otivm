
#include <stdio.h>
#include <GL/gl.h>

#include "decl.h"

void gl_print_info(void)
{
	GLint i, n;
	typedef const GLubyte* glGetStringifn(GLenum name, GLuint index);
	glGetStringifn *getStringi;

#define print_gl_string(name) printf(#name ": %s\n", (char *)glGetString(name))
	print_gl_string(GL_VERSION);
	print_gl_string(GL_SHADING_LANGUAGE_VERSION);
	print_gl_string(GL_RENDERER);
	print_gl_string(GL_VENDOR);
#undef print_gl_string

	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	getStringi = (glGetStringifn *)gl_get_proc("glGetStringi");
	for (i = 0; i < n; i++) {
		printf(
			"GL_EXTENSIONS %d: %s\n",
			i,
			(char *)getStringi(GL_EXTENSIONS, i));
	}
}

