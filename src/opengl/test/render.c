
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

#include <ok/ok.h>
#include <base/mem.h>
#include <rescache/rescache.h>

#include "../test.h"
#include "../types.h"
#include "../decl.h"
#include "../shader.h"
#include "../load-shader.h"
#include "../load-program.h"
#include "../load-geometry.h"

#define run(fn) gl_run_test(is_test_interactive() ? __func__ : NULL, fn)

static void white(void)
{
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	//glFlush();
}

static int draw_triangle_(struct gl_state *state, struct gl_test *test)
{
	static char const *shaders[] = {
		"asset/test/shader.vert",
		"asset/test/shader.frag"
	};

	struct gl_geometries const *geo;
	struct gl_program const *prog;

	geo = gl_load_geometry(&state->cache, "asset/test/triangle.obj");
	if (!geo) { fail_test("Unable to load geometry\n"); }

	prog = gl_load_program(&state->cache, shaders, length_of(shaders));
	if (!prog) { fail_test("Unable to create shader program\n"); }

	white();

	state->f.glUseProgram(prog->name);
	gl_draw_geometries(state, geo);

	gl_test_swap_buffers(test);
	if (is_test_interactive()) { gl_test_wait_for_key(test); }

	gl_release_program(&state->cache, prog);
	gl_release_geometry(&state->cache, geo);

	return ok;
}
static int draw_triangle(void) { return run(draw_triangle_); }

struct test const tests[] = {
	{ draw_triangle, "Draw a triangle" },

	{ NULL, NULL }
};

