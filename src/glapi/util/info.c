#include <stdio.h>
#include <GL/gl.h>

#include "ok/ok.h"
#include "glapi/test.h"
#include "../fwd.h"

#define print_gl_string(name) printf(#name ": %s\n", (char *)glGetString(name))

int main(void)
{
	struct gl_test *test = gl_test_make(0);

	print_gl_string(GL_VERSION);
	print_gl_string(GL_SHADING_LANGUAGE_VERSION);
	print_gl_string(GL_RENDERER);
	print_gl_string(GL_VENDOR);

	gl_test_free(test);
	return 0;
}
