
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <GL/gl.h>

#include <text/vstr.h>

#include "test.h"
#include "types.h"
#include "decl.h"

typedef void (GLAPIENTRY *debug_proc)(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	GLchar const *message,
	void const *userParam);

typedef void (GLAPIENTRY *debug_message_callback)(
	debug_proc callback,
	void const *userParam);

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
	int (*fn)(struct gl_state *state, struct gl_test *test))
{
	int status;
	struct gl_test *test;
	struct gl_state *state;

	test = gl_make_test_context(name);
	if (test) {
		state = gl_test_get_state(test);
		gl_enable_debug_output(state);
		status = fn(state, test);
		gl_disable_debug_output(state);
		gl_print_errors(__func__);
		gl_free_test_context(test);
	} else {
		status = -1;
	}
	return status;
}

static void GLAPIENTRY debug_callback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	GLchar const *message,
	void const *user_param)
{
	char const *sourcestr, *typestr;

	(void)length;
	(void)user_param;
	(void)severity;

	switch (source) {
	/* The GL */
	case GL_DEBUG_SOURCE_API_ARB: sourcestr = "GL"; break;

	/* The GLSL shader compiler or compilers for other extension-provided
	   languages */
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: sourcestr = "GLSL"; break;

	/* The window system, such as WGL or GLX */
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: sourcestr = "WS"; break;

	/* External debuggers or third-party middleware libraries */
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: sourcestr = "3RD"; break;

	/* The application */
	case GL_DEBUG_SOURCE_APPLICATION_ARB: sourcestr = "APP"; break;

	/* Sources that do not fit to any of the ones listed above */
	case GL_DEBUG_SOURCE_OTHER_ARB: sourcestr = "ETC"; break;

	default: sourcestr = "???"; break;
	}

	switch (type) {
	/* Events that generated an error */
	case GL_DEBUG_TYPE_ERROR_ARB: typestr = "error"; break;

	/* Behavior that has been marked for deprecation */
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
		typestr = "deprecated";
		break;

	/* Behavior that is undefined according to the specification */
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB: typestr = "undefined"; break;

	/* Implementation-dependent performance warnings */
	case GL_DEBUG_TYPE_PERFORMANCE_ARB: typestr = "performance"; break;

	/* Use of extensions or shaders in a way that is highly
	   vendor-specific */
	case GL_DEBUG_TYPE_PORTABILITY_ARB: typestr = "portability"; break;

	/* Types of events that do not fit any of the ones listed above */
	case GL_DEBUG_TYPE_OTHER_ARB: typestr = "other"; break;

	default: typestr = "???"; break;
	}

	fprintf(
		stderr,
		"[%s %s] %u: %s\n",
		sourcestr,
		typestr,
		id,
		(char const *)message);
}

static debug_message_callback get_debug_message_callback(void)
{
	return (debug_message_callback)gl_get_proc("glDebugMessageCallbackARB");
}

int gl_enable_debug_output(struct gl_state *state)
{
	if (!gl_is_new_extension_supported(state, "GL_ARB_debug_output")) {
		return -1;
	}
	get_debug_message_callback()(debug_callback, state);
	return 0;
}

int gl_disable_debug_output(struct gl_state *state)
{
	if (!gl_is_new_extension_supported(state, "GL_ARB_debug_output")) {
		return -1;
	}
	get_debug_message_callback()(NULL, NULL);
	return 0;
}

