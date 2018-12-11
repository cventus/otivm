#include <stdio.h>
#include <GL/gl.h>

#include "ok/ok.h"
#include "glapi/test.h"
#include "../fwd.h"

int test_make_and_ensure_context_is_current(void)
{
	struct gl_test *test = gl_test_make(0);
	struct gl_api *api = gl_test_api(test);

	if (!gl_is_current(api)) {
		printf("Not current\n");
		return -1;
	} else {
		return 0;
	}
}

int test_create_a_simple_context_and_swap_buffers(void)
{
	struct gl_test *test = gl_test_make(0);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	gl_test_swap_buffers(test);
	gl_test_free(test);

	return ok;
}

int test_extension_lookup_functions(void)
{
	char const *extensions = "abc defg hjklm";
	char const *weird_extensions = " abc   defg  hjklm ";

	/* valid extension names */
	if (!gl_find_ext(extensions, "abc")) { ok = -1; }
	if (!gl_find_ext(extensions, "defg")) { ok = -1; }
	if (!gl_find_ext(extensions, "hjklm")) { ok = -1; }
	if (!gl_find_ext(weird_extensions, "abc")) { ok = -1; }
	if (!gl_find_ext(weird_extensions, "defg")) { ok = -1; }
	if (!gl_find_ext(weird_extensions, "hjklm")) { ok = -1; }

	/* invalid extension names */
	if (gl_find_ext(extensions, "a")) { ok = -1; }
	if (gl_find_ext(extensions, "ab")) { ok = -1; }
	if (gl_find_ext(extensions, "jkl")) { ok = -1; }
	if (gl_find_ext(extensions, "xyz")) { ok = -1; }
	if (gl_find_ext(extensions, "abc ")) { ok = -1; }
	if (gl_find_ext(extensions, " defg ")) { ok = -1; }
	if (gl_find_ext(extensions, " hjklm")) { ok = -1; }
	if (gl_find_ext(extensions, "abc defg")) { ok = -1; }

	return ok;
}
