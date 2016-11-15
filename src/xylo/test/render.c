
#include <stdio.h>
#include <GL/gl.h>
#include <ok/ok.h>

#include <glapi/api.h>
#include <glapi/test.h>
#include <gm/matrix.h>

#include "../include/types.h"
#include "../private.h"

#define run(fn) gl_run_test(is_test_interactive() ? __func__ : NULL, fn)

static int creation_(struct gl_api *gl, struct gl_test *test)
{
	struct xylo *xylo = make_xylo(gl);
	if (!xylo) { return -1; }
	free_xylo(xylo);
	(void)test;
	return 0;
}
static int creation(void) { return run(creation_); }

static struct xylo_shape test_shape = {
	2,
	(struct xylo_outline[]) {
		{
			(struct xylo_leg[]) {
				{ { 0.0f, 0.5f }, { 0.5f, 0.5f }, 1.f },
				{ { 0.5f, 0.0f }, { -.1f, 0.1f }, 0.707106781f },
				{ { 0.0f, -.5f }, { -.5f, -.5f }, 0.00001f },
				{ { -.5f, 0.0f }, { -.5f, 0.5f }, 4.0f }
			},
			4
		},
		{
			(struct xylo_leg[]) {
				{ { 0.0f, 0.1f }, { 0.1f, 0.1f }, 1.f },
				{ { 0.1f, 0.0f }, { -.0f, 0.0f }, 0.707106781f },
				{ { 0.0f, -.1f }, { -.1f, -.1f }, 0.00001f },
				{ { -.1f, 0.0f }, { -.1f, 0.1f }, 4.0f }
			},
			4
		}
	}
};

static int draw_(struct gl_api *gl, struct gl_test *test)
{
	struct xylo *xylo = make_xylo(gl);
	struct xylo_glshape_set *set;
	struct xylo_glshape *shape;
	float transform[16], proj[16], to_clip[16], to_world[16];

	if (!xylo) { return -1; }

	set = xylo_make_glshape_set(gl, 1, &test_shape);
	if (!set) { return -1; }
	shape = xylo_get_glshape(set, 0);
	if (!shape) { return -1; }

	/* setup matrices */
	m44translatef(to_world, -0.3f, -0.3f, 0.f);
	m44translatef(transform, 0.f, 0.f, -1.f);
	m44perspectivef(proj, 1.570796327, 1.0f, 0.4f, 10.0f);
	m44mulf(to_clip, proj, transform);

	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	xylo_begin(xylo);
	xylo_set_shape_set(xylo, set);
	xylo_set_to_world(xylo, to_world);
	xylo_set_to_clip(xylo, to_clip);
	xylo_draw_shape(xylo, shape);
	xylo_end(xylo);

	if (xylo_free_glshape_set(set, gl)) { return -1; }
	gl_test_swap_buffers(test);

	if (is_test_interactive()) { gl_test_wait_for_key(test); }
	free_xylo(xylo);
	return 0;
}
static int draw(void) { return run(draw_); }

struct test const tests[] = {
	{ creation, "create xylo renderer" },
	{ draw, "simple render" },

	{ NULL, NULL }
};

