#include <stdio.h>
#include <GL/gl.h>
#include <ok/ok.h>

#include <glapi/api.h>
#include <glapi/test.h>
#include <gm/matrix.h>
#include <base/gbuf.h>

#include "../include/xylo.h"
#include "../include/types.h"
#include "../include/draw.h"
#include "../include/shape.h"
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
			4,
			(struct xylo_leg[]) {
				{ { 0.0f, 0.5f }, { 0.5f, 0.5f }, 0.707106781f },
				{ { 0.5f, 0.0f }, { 0.5f, 0.0f }, 1.f },
				{ { 0.0f, -.5f }, { -.5f, -.5f }, 0.00001f },
				{ { -.5f, 0.0f }, { -.5f, 0.5f }, 4.0f }
			}
		},
		{
			4,
			(struct xylo_leg[]) {
				{ { 0.0f, 0.1f }, { 0.1f, 0.1f }, 1.f },
				{ { 0.1f, 0.0f }, { -.0f, 0.0f }, 0.707106781f },
				{ { 0.0f, -.1f }, { -.1f, -.1f }, 0.00001f },
				{ { -.1f, 0.0f }, { -.1f, 0.1f }, 4.0f }
			}
		}
	}
};

static int draw_(struct gl_api *gl, struct gl_test *test)
{
	struct xylo *xylo = make_xylo(gl);
	struct xylo_glshape_set *set;
	struct xylo_glshape const *shape;
	float transform[16], proj[16], to_clip[16], to_world[16];

	if (!xylo) { return -1; }

	set = xylo_make_glshape_set(gl, 1, &test_shape);
	if (!set) { return -1; }
	shape = xylo_get_glshape(set, 0);
	if (!shape) { return -1; }

	/* setup matrices */
	m44translatef(to_world, 0.f, 0.f, 0.f);
	m44translatef(transform, 0.f, 0.f, -1.f);
	m44perspectivef(proj, 1.570796327, 1.0f, 0.4f, 10.0f);
	m44mulf(to_clip, proj, transform);

	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	xylo_begin(xylo);
	xylo_set_shape_set(xylo, set);
	xylo_set_to_world(xylo, to_world);
	xylo_set_to_clip(xylo, to_clip);
	xylo_draw_glshape(xylo, shape);
	xylo_end(xylo);

	if (xylo_free_glshape_set(set, gl)) { return -1; }
	gl_test_swap_buffers(test);

	if (is_test_interactive()) { gl_test_wait_for_key(test); }
	free_xylo(xylo);
	return 0;
}
static int draw(void) { return run(draw_); }

static int dlist_(struct gl_api *gl, struct gl_test *test)
{
	struct xylo *xylo = make_xylo(gl);
	struct xylo_glshape_set *set;
	struct xylo_glshape const *shape;
	float transform[16], proj[16], to_clip[16];
	struct xylo_dlist dlist;
	struct xylo_dshape a, b, c;

	if (!xylo) { return -1; }
	set = xylo_make_glshape_set(gl, 1, &test_shape);
	if (!set) { return -1; }
	shape = xylo_get_glshape(set, 0);
	if (!shape) { return -1; }

	/* create list nodes */
	xylo_init_dshape(&a, shape);
	xylo_init_dshape(&b, shape);
	xylo_init_dshape(&c, shape);

	/* create draw list */
	xylo_init_dlist(&dlist);

	xylo_dlist_append(&dlist, &a.header);
	xylo_dlist_append(&dlist, &b.header);
	xylo_dlist_append(&dlist, &c.header);

	m22mulsf(a.m22, a.m22, 0.1f);
	m22mulsf(b.m22, b.m22, 0.2f);
	m22mulsf(c.m22, c.m22, 0.3f);

	a.pos[0] = a.pos[1] = -0.3;
	c.pos[0] = b.pos[1] = 0.3;

	/* setup matrices */
	m44translatef(transform, 0.f, 0.f, -1.f);
	m44perspectivef(proj, 1.570796327, 1.0f, 0.4f, 10.0f);
	m44mulf(to_clip, proj, transform);

	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	xylo_begin(xylo);
	xylo_set_shape_set(xylo, set);
	xylo_set_to_clip(xylo, to_clip);
	xylo_draw(xylo, &dlist.header);
	xylo_end(xylo);

	xylo_term_dlist(&dlist);
	xylo_term_dshape(&a);
	xylo_term_dshape(&b);
	xylo_term_dshape(&c);
	if (xylo_free_glshape_set(set, gl)) { return -1; }
	gl_test_swap_buffers(test);

	if (is_test_interactive()) { gl_test_wait_for_key(test); }
	free_xylo(xylo);
	return 0;
}
static int dlist(void) { return run(dlist_); }

struct test const tests[] = {
	{ creation, "create xylo renderer" },
	{ draw, "simple render" },
	{ dlist, "render draw list" },

	{ NULL, NULL }
};
