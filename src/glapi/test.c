
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <GL/gl.h>

#include <text/vstr.h>

#include "include/dbgmsg.h"
#include "include/test.h"

#include "fwd.h"

char const *gl_strerror(GLenum error)
{
#define enumcase(name) case name: return #name; break;
	switch (error) {
	enumcase(GL_NO_ERROR)
	enumcase(GL_INVALID_ENUM)
	enumcase(GL_INVALID_VALUE)
	enumcase(GL_INVALID_OPERATION)
	enumcase(GL_INVALID_FRAMEBUFFER_OPERATION)
	enumcase(GL_OUT_OF_MEMORY)
	enumcase(GL_STACK_UNDERFLOW)
	enumcase(GL_STACK_OVERFLOW)
	default: return "unknown";
	}
#undef enumcase
}

void gl_fprintf_errors(FILE *fp, char const *fmt, ...)
{
	va_list ap;
	char *msg;
	char const *prompt, *colon, *errstr;
	GLenum error;

	if (fmt) {
		va_start(ap, fmt);
		msg = vstrfmt(0, 0, fmt, ap);
		va_end(ap);
	} else {
		msg = NULL;
	}
	while (error = glGetError(), error != GL_NO_ERROR) {
		prompt = msg ? msg : "";
		colon = msg ? ": " : "";
		errstr = gl_strerror(error);
		(void)fprintf(fp, "%s%s%s\n", prompt, colon, errstr);
	}
	free(msg);
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

	(void)printf(
		"[%s %s] %u: %s\n",
		sourcestr,
		typestr,
		id,
		(char const *)message);
}

int gl_run_test(
	char const *name,
	int (*fn)(struct gl_state *state, struct gl_test *test))
{
	int status;
	struct gl_test *test;
	struct gl_state *state;
	struct gl_dbgmsg const *dbgmsg;

	test = gl_test_make(name);
	if (test) {
		state = gl_test_state(test);
		dbgmsg = gl_get_dbgmsg(state);
		if (dbgmsg) {
			dbgmsg->DebugMessageCallbackARB(debug_callback, state);
		}
		status = fn(state, test);
		if (dbgmsg) {
			dbgmsg->DebugMessageCallbackARB(NULL, NULL);
		}
		gl_fprintf_errors(stdout, "%s", name ? name : __func__);
		gl_test_free(test);
	} else {
		status = -1;
	}
	return status;
}

