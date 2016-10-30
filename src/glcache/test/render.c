
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

#include <ok/ok.h>
#include <base/mem.h>
#include <rescache/rescache.h>

#include <opengl/opengl.h>
#include <opengl/test.h>
#include <opengl/core.h>
#include "../include/cache.h"
#include "../include/types.h"
#include "../private.h"
#include "../decl.h"
#include "../shader.h"

#define run(fn) gl_run_test(is_test_interactive() ? __func__ : NULL, fn)

static void white(void)
{
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
}

static int draw_triangle_(struct gl_state *state, struct gl_test *test)
{
	static char const *shaders[] = {
		"asset/test/shader.vert",
		"asset/test/shader.frag"
	};

	struct gl_cache *cache;
	struct gl_geometries const *geo;
	struct gl_program const *prog;
	struct gl_core const *core;

	core = gl_get_core(state);
	cache = gl_make_cache(state);
	if (!cache) { fail_test("Unable to create cache\n"); }

	geo = gl_load_geometry(cache, "asset/test/triangle.obj");
	if (!geo) { fail_test("Unable to load geometry\n"); }

	prog = gl_load_program(cache, shaders, length_of(shaders));
	if (!prog) { fail_test("Unable to create shader program\n"); }

	white();

	core->UseProgram(prog->name);
	gl_draw_geometries(state, geo);

	gl_test_swap_buffers(test);
	if (is_test_interactive()) { gl_test_wait_for_key(test); }

	gl_release_program(cache, prog);
	gl_release_geometry(cache, geo);

	gl_free_cache(cache);

	return ok;
}
static int draw_triangle(void) { return run(draw_triangle_); }

struct test const tests[] = {
	{ draw_triangle, "Draw a triangle" },

	{ NULL, NULL }
};

