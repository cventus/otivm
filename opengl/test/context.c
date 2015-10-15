
#include <stdio.h>
#include <GL/gl.h>
#include <ok/ok.h>

#include "../test.h"
#include "../types.h"
#include "../decl.h"

int info(void)
{
	struct gltest *test = gl_make_test_context(0);
	gl_print_info();
	gl_free_test_context(test);
	return 0;
}

int make_context(void)
{
	struct gltest *test = gl_make_test_context(0);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	gl_test_swap_buffers(test);
	gl_free_test_context(test);

	return ok;
}

int extensions(void)
{
	const char *extensions = "abc defg hjklm";
	const char *weird_extensions = " abc   defg  hjklm ";

	/* valid extension names */
	if (!gl_is_extension_supported(extensions, "abc")) { ok = -1; }
	if (!gl_is_extension_supported(extensions, "defg")) { ok = -1; }
	if (!gl_is_extension_supported(extensions, "hjklm")) { ok = -1; }
	if (!gl_is_extension_supported(weird_extensions, "abc")) { ok = -1; }
	if (!gl_is_extension_supported(weird_extensions, "defg")) { ok = -1; }
	if (!gl_is_extension_supported(weird_extensions, "hjklm")) { ok = -1; }

	/* invalid extension names */
	if (gl_is_extension_supported(extensions, "a")) { ok = -1; }
	if (gl_is_extension_supported(extensions, "ab")) { ok = -1; }
	if (gl_is_extension_supported(extensions, "jkl")) { ok = -1; }
	if (gl_is_extension_supported(extensions, "xyz")) { ok = -1; }
	if (gl_is_extension_supported(extensions, "abc ")) { ok = -1; }
	if (gl_is_extension_supported(extensions, " defg ")) { ok = -1; }
	if (gl_is_extension_supported(extensions, " hjklm")) { ok = -1; }
	if (gl_is_extension_supported(extensions, "abc defg")) { ok = -1; }

	return ok;
}

struct test const tests[] = {
	{ info, "display opengl context information" },
	{ make_context, "create a simple context" },
	{ extensions, "is extension supported" },

	{ NULL, NULL }
};

