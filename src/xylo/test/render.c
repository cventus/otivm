#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <ok/ok.h>

#include <glapi/api.h>
#include <glapi/test.h>
#include <gm/matrix.h>
#include <base/gbuf.h>
#include <base/mem.h>

#include <tempo/tempo.h>

#include "../include/xylo.h"
#include "../include/types.h"
#include "../include/draw.h"
#include "../include/tgraph.h"
#include "../include/shape.h"
#include "../private.h"

#define run(fn) gl_run_test(is_test_interactive() ? __func__ : NULL, fn)

static float const red[4] = { 0.87, 0.05, 0.11, 1.0 };
static float const black[4] = { 0.02, 0.02, 0.02, 1.0 };

static struct xylo_shape test_shape[] = {
	{
		1,
		(struct xylo_outline[]) {
			{
				21,
				(struct xylo_leg[]) {
					{ { 0.20f,-0.50f }, { 0.02f,-0.45f }, 0.707106781f },
					{ { 0.02f,-0.24f }, { 0.02f,-0.24f }, 0.f },
					{ { 0.07f,-0.24f }, { 0.07f,-0.37f }, 0.707106781f },

					{ { 0.22f,-0.37f }, { 0.37f,-0.37f }, 0.707106781f },
					{ { 0.37f,-0.22f }, { 0.37f,-0.07f }, 0.707106781f },
					{ { 0.22f,-0.07f }, { 0.07f,-0.07f }, 0.707106781f },

					{ { 0.07f,-0.20f }, { 0.07f,-0.20f }, 0.f },
					{ { 0.02f,-0.20f }, { 0.02f,-0.20f }, 0.f },

					{ { 0.02f,-0.10f }, { 0.15f,-0.10f }, 0.707106781f },
					{ { 0.15f, 0.05f }, { 0.15f, 0.20f }, 0.707106781f },
					{ { 0.00f, 0.20f }, {-0.15f, 0.20f }, 0.707106781f },
					{ {-0.15f, 0.05f }, {-0.15f,-0.10f }, 0.707106781f },

					{ {-0.02f,-0.10f }, {-0.02f,-0.10f }, 0.f },
					{ {-0.02f,-0.20f }, {-0.02f,-0.20f }, 0.f },

					{ {-0.07f,-0.20f }, {-0.07f,-0.07f }, 0.707106781f },


					{ {-0.22f,-0.07f }, {-0.37f,-0.07f }, 0.707106781f },
					{ {-0.37f,-0.22f }, {-0.37f,-0.37f }, 0.707106781f },
					{ {-0.22f,-0.37f }, {-0.07f,-0.37f }, 0.707106781f },

					{ {-0.07f,-0.24f }, {-0.07f,-0.24f }, 0.f },

					{ {-0.02f,-0.24f }, {-0.02f,-0.45f }, 0.707106781f },
					{ {-0.20f,-0.50f }, { 0.00f,-0.53f }, 0.707106781f },
				}
			}
		}
	},
	{
		1,
		(struct xylo_outline[]) {
			{
				4,
				(struct xylo_leg[]) {
					{ {-0.13f, 0.02f }, { 0.00f, 0.24f }, 4.0f },
					{ { 0.13f, 0.02f }, { 0.22f,-0.16f }, 4.0f },
					{ { 0.13f,-0.34f }, { 0.00f,-0.56f }, 4.0f },
					{ {-0.13f,-0.34f }, {-0.22f,-0.16f }, 4.0f }
				}
			}
		}
	},
	{
		1,
		(struct xylo_outline[]) {
			{
				11,
				(struct xylo_leg[]) {
					{ { 0.20f,-0.50f }, { 0.02f,-0.45f }, 0.707106781f },
					{ { 0.02f,-0.29f }, { 0.10f,-0.40f }, 0.707106781f },
					{ { 0.20f,-0.40f }, { 0.40f,-0.40f }, 0.707106781f },
					{ { 0.40f,-0.20f }, { 0.40f,-0.00f }, 1.0f },
					{ { 0.20f, 0.15f }, { 0.00f, 0.30f }, 1.0f },
					{ { 0.00f, 0.35f }, {-0.00f, 0.30f }, 1.0f },
					{ {-0.20f, 0.15f }, {-0.40f,-0.00f }, 1.0f },
					{ {-0.40f,-0.20f }, {-0.40f,-0.40f }, 0.707106781f },
					{ {-0.20f,-0.40f }, {-0.10f,-0.40f }, 0.707106781f },
					{ {-0.02f,-0.29f }, {-0.02f,-0.45f }, 0.707106781f },
					{ {-0.20f,-0.50f }, { 0.00f,-0.53f }, 0.707106781f },
				}
			}
		}
	},
	{
		1,
		(struct xylo_outline[]) {
			{
				8,
				(struct xylo_leg[]) {
					{ { 0.0f, 0.05f }, { 0.0f, 0.25f }, 0.707106781f },
					{ { 0.2f, 0.25f }, { 0.4f, 0.25f }, 0.707106781f },
					{ { 0.4f, 0.05f }, { 0.4f,-0.15f }, 1.0f },
					{ { 0.2f,-0.30f }, { 0.0f,-0.45f }, 1.0f },
					{ { 0.0f,-0.50f }, {-0.0f,-0.45f }, 1.0f },
					{ {-0.2f,-0.30f }, {-0.4f,-0.15f }, 1.0f },
					{ {-0.4f, 0.05f }, {-0.4f, 0.25f }, 0.707106781f },
					{ {-0.2f, 0.25f }, { 0.0f, 0.25f }, 0.707106781f },
				}
			}
		}
	},
	{
		2,
		(struct xylo_outline[]) {
			{
				4,
				(struct xylo_leg[]) {
					{ { 0.0f, 0.5f }, { 0.5f, 0.5f }, 0.707106781f },
					{ { 0.5f, 0.0f }, { 0.0f, 0.0f }, 1.f },
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
	}
};

static int creation_(struct gl_api *gl, struct gl_test *test)
{
	struct xylo *xylo = make_xylo(gl);
	if (!xylo) { return -1; }
	free_xylo(xylo);
	(void)test;
	return 0;
}
static int creation(void) { return run(creation_); }

static int draw_(struct gl_api *gl, struct gl_test *test)
{
	struct xylo *xylo = make_xylo(gl);
	struct xylo_glshape_set *set;
	struct xylo_glshape const *shape;
	float transform[16], proj[16], to_clip[16], to_world[16], mvp[16];

	if (!xylo) { return -1; }
	set = xylo_make_glshape_set(gl, length_of(test_shape), test_shape);
	if (!set) { return -1; }
	shape = xylo_get_glshape(set, 0);
	if (!shape) { return -1; }

	/* setup matrices */
	m44translatef(to_world, 0.f, 0.f, 0.f);
	m44translatef(transform, 0.f, 0.f, -1.f);
	m44perspectivef(proj, 1.570796327, 1.0f, 0.4f, 10.0f);
	m44mulf(to_clip, proj, transform);
	m44mulf(mvp, to_clip, to_world);

	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	xylo_begin(xylo);
	xylo_set_shape_set(xylo, set);
	xylo_set_mvp(xylo, mvp);
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
	struct xylo_dlist dlist;
	struct xylo_dshape a, b, c;

	if (!xylo) { return -1; }
	set = xylo_make_glshape_set(gl, length_of(test_shape), test_shape);
	if (!set) { return -1; }

	/* create list nodes */
	xylo_init_dshape(&a, black, xylo_get_glshape(set, 0));
	xylo_init_dshape(&b, black, xylo_get_glshape(set, 2));
	xylo_init_dshape(&c, red, xylo_get_glshape(set, 3));

	/* create draw list */
	xylo_init_dlist(&dlist);
	xylo_dlist_append(&dlist, &a.draw);
	xylo_dlist_append(&dlist, &b.draw);
	xylo_dlist_append(&dlist, &c.draw);

	m22mulsf(a.m22, a.m22, 0.1f);
	m22mulsf(b.m22, b.m22, 0.2f);
	m22mulsf(c.m22, c.m22, 0.3f);

	a.pos[0] = a.pos[1] = -0.3;
	c.pos[0] = b.pos[1] = 0.3;

	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	xylo_begin(xylo);
	xylo_set_shape_set(xylo, set);
	xylo_draw(xylo, &dlist.draw);
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

static void copy_transform(struct xylo_tnode *tnode, struct xylo_dshape *shape)
{
	float const *t;
	t = xylo_tnode_global(tnode);
	(void)memcpy(shape->m22, t, 2 * sizeof *t);
	(void)memcpy(shape->m22 + 2, t + 3, 2 * sizeof *t);
	(void)memcpy(shape->pos, t + 6, sizeof shape->pos);
}

static void update(void *dest, void const *child, void const *parent)
{
	(void)m33mulf(dest, parent, child);
}

static int transformed_(struct gl_api *gl, struct gl_test *test)
{
	struct xylo *xylo;
	struct xylo_glshape_set *set;
	struct xylo_glshape const *clubs, *diamonds, *spades, *hearts;
	struct xylo_dlist dlist;
	struct xylo_dshape a, b, c;
	struct xylo_tgraph *tgraph;
	struct xylo_tnode *root, *a_t[2], *b_t[1], *c_t[3];
	struct pfclock *clk;
	float id[9] = { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f };
	float *p;
	struct stopwatch sw;
	double dt;

	clk = pfclock_make();
	if (!clk) { return -1; }
	xylo = make_xylo(gl);
	if (!xylo) { return -1; }
	set = xylo_make_glshape_set(gl, length_of(test_shape), test_shape);
	if (!set) { return -1; }
	clubs = xylo_get_glshape(set, 0);
	if (!clubs) { return -1; }
	diamonds = xylo_get_glshape(set, 1);
	if (!diamonds) { return -1; }
	spades = xylo_get_glshape(set, 2);
	if (!spades) { return -1; }
	hearts = xylo_get_glshape(set, 3);
	if (!hearts) { return -1; }

	/* create list nodes */
	xylo_init_dshape(&a, black, clubs);
	xylo_init_dshape(&b, red, hearts);
	xylo_init_dshape(&c, red, diamonds);

	/* create draw list */
	xylo_init_dlist(&dlist);

	xylo_dlist_append(&dlist, &a.draw);
	xylo_dlist_append(&dlist, &b.draw);
	xylo_dlist_append(&dlist, &c.draw);

	/* create transformation graph */
	tgraph = xylo_make_tgraph(sizeof id);
	root = xylo_make_tnode(tgraph, NULL, id);
	a_t[0] = xylo_make_tnode(tgraph, root, id);
	a_t[1] = xylo_make_tnode(tgraph, a_t[0], id);
	b_t[0] = xylo_make_tnode(tgraph, a_t[0], id);
	c_t[0] = xylo_make_tnode(tgraph, root, id);
	c_t[1] = xylo_make_tnode(tgraph, c_t[0], id);
	c_t[2] = xylo_make_tnode(tgraph, c_t[1], id);

	p = xylo_tnode_local(b_t[0]);
	p[0] = 0.22f;
	p[4] = 0.33f;
	p = xylo_tnode_local(c_t[1]);
	p[0] = 0.3f;
	p[4] = 0.3f;
	p[6] = 0.3f;

	glClearColor(1.f, 1.f, 1.f, 1.f);

	stopwatch_start(&sw, pfclock_usec(clk));
	xylo_begin(xylo);
	xylo_set_shape_set(xylo, set);
	do {
		dt = stopwatch_elapsed(&sw, pfclock_usec(clk)) * 1.e-6;

		/* animate nodes a bit*/
		p = xylo_tnode_local(root);
		p[6] = cos(dt) * 0.4f;
		p[7] = sin(dt) * 0.4f;

		p = xylo_tnode_local(a_t[0]);
		m33rotzf(p, dt * -0.7f + 0.1f);

		/* make a orbit b */
		p = xylo_tnode_local(a_t[1]);
		m33rotzf(p, -2.1f * dt);
		p[0] *= 0.1f; p[1] *= 0.1f;
		p[3] *= 0.1f; p[4] *= 0.1f;
		p[6] = 0.3*sin(2.1f * dt);
		p[7] = 0.3*cos(2.1f * dt);

		p = xylo_tnode_local(c_t[0]);
		m33rotzf(p, -dt);
		p[6] = 0.2f + sin(dt * 0.73) * 0.1f;
		p[7] = 0.2f + cos(dt * 0.73) * 0.1f;
		p = xylo_tnode_local(c_t[2]);
		m33rotzf(p, dt);

		/* translate */
		xylo_tgraph_transform(tgraph, update);
		copy_transform(a_t[1], &a);
		copy_transform(b_t[0], &b);
		copy_transform(c_t[2], &c);

		/* draw */
		glClear(GL_COLOR_BUFFER_BIT);
		xylo_draw(xylo, &dlist.draw);
		gl_test_swap_buffers(test);
	} while (gl_test_poll_key(test) == 0);
	xylo_end(xylo);

	xylo_term_dlist(&dlist);
	xylo_term_dshape(&a);
	xylo_term_dshape(&b);
	xylo_term_dshape(&c);
	if (xylo_free_glshape_set(set, gl)) { return -1; }
	free_xylo(xylo);
	pfclock_free(clk);
	return 0;
}
static int transformed(void) { return run(transformed_); }

struct test const tests[] = {
	{ creation, "create xylo renderer" },
	{ draw, "simple render" },
	{ dlist, "render draw list" },
	{ transformed, "render items with tgraph" },

	{ NULL, NULL }
};
