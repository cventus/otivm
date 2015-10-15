
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <GL/gl.h>

#include <text/vstr.h>

#include "test.h"
#include "types.h"
#include "decl.h"

void gl_print_errors(char const *fmt, ...)
{
	va_list ap;
	char *msg;

	va_start(ap, fmt);
	msg = vstrfmt(0, 0, fmt, ap);
	va_end(ap);

	do switch (glGetError()) {
	case GL_NO_ERROR: break;

	case GL_INVALID_ENUM:
		printf("%s: GL_INVALID_ENUM\n", msg);
		continue;

	case GL_INVALID_VALUE:
		printf("%s: GL_INVALID_VALUE\n", msg);
		continue;

	case GL_INVALID_OPERATION:
		printf("%s: GL_INVALID_OPERATION\n", msg);
		continue;

	case GL_INVALID_FRAMEBUFFER_OPERATION:
		printf("%s: GL_INVALID_FRAMEBUFFER_OPERATION\n", msg);
		continue;

	case GL_OUT_OF_MEMORY:
		printf("%s: GL_OUT_OF_MEMORY\n", msg);
		continue;

	case GL_STACK_UNDERFLOW:
		printf("%s: GL_STACK_UNDERFLOW\n", msg);
		continue;

	case GL_STACK_OVERFLOW:
		printf("%s: GL_STACK_OVERFLOW\n", msg);
		continue;

	default: break;
	} while (0);

	free(msg);
}

int gl_run_test(
	char const *name,
	int (*fn)(struct glstate *state, struct gltest *test))
{
	int status;
	struct gltest *test;
	struct glstate *state;

	test = gl_make_test_context(name);
	if (test) {
		state = gl_test_get_state(test);
		status = fn(state, test);
		gl_print_errors(__func__);
		gl_free_test_context(test);
	} else {
		status = -1;
	}
	return status;
}

