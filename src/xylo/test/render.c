#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <ok/ok.h>
#include <glapi/api.h>
#include <glapi/core.h>
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
					{ { 0.20f, -0.30f }, { 0.02f, -0.25f }, 0.707106781f },
					{ { 0.02f, -0.04f }, { 0.02f, -0.04f }, 0.f },
					{ { 0.07f, -0.04f }, { 0.07f, -0.17f }, 0.707106781f },
					{ { 0.22f, -0.17f }, { 0.37f, -0.17f }, 0.707106781f },
					{ { 0.37f, -0.02f }, { 0.37f,  0.12f }, 0.707106781f },
					{ { 0.22f,  0.12f }, { 0.07f,  0.12f }, 0.707106781f },
					{ { 0.07f, -0.00f }, { 0.07f, -0.00f }, 0.f },
					{ { 0.02f, -0.00f }, { 0.02f, -0.00f }, 0.f },
					{ { 0.02f,  0.09f }, { 0.15f,  0.09f }, 0.707106781f },
					{ { 0.15f,  0.24f }, { 0.15f,  0.39f }, 0.707106781f },
					{ { 0.00f,  0.39f }, {-0.15f,  0.39f }, 0.707106781f },
					{ {-0.15f,  0.24f }, {-0.15f,  0.09f }, 0.707106781f },
					{ {-0.02f,  0.09f }, {-0.02f,  0.09f }, 0.f },
					{ {-0.02f, -0.00f }, {-0.02f, -0.00f }, 0.f },
					{ {-0.07f, -0.00f }, {-0.07f,  0.12f }, 0.707106781f },
					{ {-0.22f,  0.12f }, {-0.37f,  0.12f }, 0.707106781f },
					{ {-0.37f, -0.02f }, {-0.37f, -0.17f }, 0.707106781f },
					{ {-0.22f, -0.17f }, {-0.07f, -0.17f }, 0.707106781f },
					{ {-0.07f, -0.04f }, {-0.07f, -0.04f }, 0.f },
					{ {-0.02f, -0.04f }, {-0.02f, -0.25f }, 0.707106781f },
					{ {-0.20f, -0.30f }, { 0.00f, -0.33f }, 0.707106781f },
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
					{ {-0.125f, 0.22f }, { 0.00f, 0.44f }, 3.6f },
					{ { 0.125f, 0.22f }, { 0.24f,-0.00f }, 4.0f },
					{ { 0.125f,-0.22f }, { 0.00f,-0.44f }, 3.6f },
					{ {-0.125f,-0.22f }, {-0.24f,-0.00f }, 4.0f }
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
					{ { 0.20f,-0.29f }, { 0.02f,-0.24f }, 0.707106781f },
					{ { 0.02f,-0.08f }, { 0.10f,-0.19f }, 0.707106781f },
					{ { 0.20f,-0.19f }, { 0.40f,-0.19f }, 0.707106781f },
					{ { 0.40f, 0.00f }, { 0.40f, 0.20f }, 1.0f },
					{ { 0.20f, 0.35f }, { 0.00f, 0.50f }, 1.0f },
					{ { 0.00f, 0.55f }, {-0.00f, 0.50f }, 1.0f },
					{ {-0.20f, 0.35f }, {-0.40f, 0.20f }, 1.0f },
					{ {-0.40f, 0.00f }, {-0.40f,-0.19f }, 0.707106781f },
					{ {-0.20f,-0.19f }, {-0.10f,-0.19f }, 0.707106781f },
					{ {-0.02f,-0.08f }, {-0.02f,-0.24f }, 0.707106781f },
					{ {-0.20f,-0.29f }, { 0.00f,-0.32f }, 0.707106781f },
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
					{ { 0.0f, 0.09f }, { 0.0f, 0.29f }, 0.707106781f },
					{ { 0.2f, 0.29f }, { 0.4f, 0.29f }, 0.707106781f },
					{ { 0.4f, 0.09f }, { 0.4f,-0.11f }, 1.0f },
					{ { 0.2f,-0.26f }, { 0.0f,-0.41f }, 1.0f },
					{ { 0.0f,-0.46f }, {-0.0f,-0.41f }, 1.0f },
					{ {-0.2f,-0.26f }, {-0.4f,-0.11f }, 1.0f },
					{ {-0.4f, 0.09f }, {-0.4f, 0.29f }, 0.707106781f },
					{ {-0.2f, 0.29f }, { 0.0f, 0.29f }, 0.707106781f },
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

static float const view_width = 640.f;
static float const view_height = 480.f;

static void ortho(float *dest, GLint const size[2])
{
	float left, right, bottom, top;
	float aspect, v_aspect, width, height;

	v_aspect = view_width / view_height;
	if (size[0] <= 0 || size[1] <= 0) {
		aspect = 1.0;
	} else {
		aspect = (float)size[0] / (float)size[1];
	}
	if (aspect > v_aspect) {
		/* more portrait like */
		height = view_height;
		width = view_width * aspect / v_aspect;
	} else {
		/* more landscape like, or square */
		height = view_height * v_aspect / aspect;
		width = view_width;
	}
	left = -width * 0.5f;
	right = width * 0.5f;
	bottom = -height * 0.5f;
	top = height * 0.5f;
	(void)m44orthographicf(dest, left, right, bottom, top, 0.f, 100.f);
}

static void update_view(
	struct gl_core33 const *restrict gl,
	struct xylo_view *view)
{
	GLint size[] = { gl_test_output_width, gl_test_output_height };

	gl->Viewport(0, 0, size[0], size[1]);
	ortho(view->projection, size);
}

static int creation_(struct gl_api *api, struct gl_test *test)
{
	struct xylo *xylo = make_xylo(api);
	if (!xylo) { return -1; }
	free_xylo(xylo);
	(void)test;
	return 0;
}
static int creation(void) { return run(creation_); }

static int dlist_(struct gl_api *api, struct gl_test *test)
{
	struct xylo *xylo;
	struct xylo_glshape_set *set;
	struct xylo_dlist dlist;
	struct xylo_dshape a, b, c;
	struct gl_core33 const *gl;
	struct xylo_view view;

	gl = gl_get_core33(api);
	if (!gl) { skip_test("OpenGL 3.3 or above required"); }
	xylo = make_xylo(api);
	if (!xylo) { return -1; }
	set = xylo_make_glshape_set(api, length_of(test_shape), test_shape);
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

	m22mulsf(a.m22, a.m22, 30.0f);
	m22mulsf(b.m22, b.m22, 60.0f);
	m22mulsf(c.m22, c.m22, 90.0f);

	a.pos[0] = a.pos[1] = -90.0;
	c.pos[0] = b.pos[1] = 90.0;

	gl->ClearColor(1.f, 1.f, 1.f, 1.f);
	gl->Clear(GL_COLOR_BUFFER_BIT);

	update_view(gl, &view);
	xylo_set_shape_set(xylo, set);
	xylo_draw(xylo, &view, &dlist.draw);

	xylo_term_dlist(&dlist);
	xylo_term_dshape(&a);
	xylo_term_dshape(&b);
	xylo_term_dshape(&c);
	if (xylo_free_glshape_set(set, api)) { return -1; }
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

static int transformed_(struct gl_api *api, struct gl_test *test)
{
	struct xylo *xylo;
	struct xylo_glshape_set *set;
	struct xylo_glshape const *clubs, *diamonds, *spades, *hearts;
	struct xylo_dlist dlist;
	struct xylo_dshape a, b, c;
	struct xylo_tgraph *tgraph;
	struct xylo_tnode *root, *a_t[2], *b_t[1], *c_t[3];
	struct pfclock *clk;
	struct gl_core33 const *gl;
	float id[9] = { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f };
	float *p;
	struct stopwatch sw;
	double dt;
	struct xylo_view view;

	clk = pfclock_make();
	if (!clk) { return -1; }
	gl = gl_get_core33(api);
	if (!gl) { skip_test("OpenGL 3.3 or above required"); }
	xylo = make_xylo(api);
	if (!xylo) { return -1; }
	set = xylo_make_glshape_set(api, length_of(test_shape), test_shape);
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
	p[0] = 66.f;
	p[4] = 99.f;
	p = xylo_tnode_local(c_t[1]);
	p[0] = 90.f;
	p[4] = 90.f;
	p[6] = 90.f;

	gl->ClearColor(1.f, 1.f, 1.f, 1.f);

	stopwatch_start(&sw, pfclock_usec(clk));
	xylo_begin(xylo);
	xylo_set_shape_set(xylo, set);
	do {
		dt = stopwatch_elapsed(&sw, pfclock_usec(clk)) * 1.e-6;

		/* animate nodes a bit*/
		p = xylo_tnode_local(root);
		p[6] = cos(dt) * 120.f;
		p[7] = sin(dt) * 120.f;

		p = xylo_tnode_local(a_t[0]);
		m33rotzf(p, dt * -0.7f + 0.1f);

		/* make a orbit b */
		p = xylo_tnode_local(a_t[1]);
		m33rotzf(p, -2.1f * dt);
		p[0] *= 30.f; p[1] *= 30.f;
		p[3] *= 30.f; p[4] *= 30.f;
		p[6] = 90.f*sin(2.1f * dt);
		p[7] = 90.f*cos(2.1f * dt);

		p = xylo_tnode_local(c_t[0]);
		m33rotzf(p, -dt);
		p[6] = 60.f + sin(dt * 0.73) * 30.f;
		p[7] = 60.f + cos(dt * 0.73) * 30.f;
		p = xylo_tnode_local(c_t[2]);
		m33rotzf(p, dt);

		/* translate */
		xylo_tgraph_transform(tgraph, update);
		copy_transform(a_t[1], &a);
		copy_transform(b_t[0], &b);
		copy_transform(c_t[2], &c);

		/* draw */
		gl->Clear(GL_COLOR_BUFFER_BIT);
		update_view(gl, &view);
		xylo_draw(xylo, &view, &dlist.draw);
		gl_test_swap_buffers(test);
		gl->Finish();
	} while (gl_test_poll_key(test) == 0);
	xylo_end(xylo);

	xylo_term_dlist(&dlist);
	xylo_term_dshape(&a);
	xylo_term_dshape(&b);
	xylo_term_dshape(&c);
	if (xylo_free_glshape_set(set, api)) { return -1; }
	free_xylo(xylo);
	pfclock_free(clk);
	return 0;
}
static int transformed(void) { return run(transformed_); }

static void update_transform(size_t i, double t, struct xylo_dshape *shape)
{
	double c, s, scale, phase, avel, xvel, yvel;
	double range = 360.0;

	avel = 1.0;
	phase = i;

	if (i % 50 == 0) {
		scale = 42.0;
	} else if (i % 13 == 0) {
		scale = 85.0;
	} else if (i % 10 == 0) {
		scale = 15.0;
	} else {
		scale = 30.0;
	}
	if (i % 11 == 0) {
		avel = -0.5;
	} else {
		avel = 1.0;
	}
	if (i % 3 == 0) {
		xvel = 133.0;
	} else if (i % 7 == 0) {
		xvel = 150.0;
	} else {
		xvel = 100.0;
	}
	if ((i + 2) % 3 == 0) {
		yvel = 150.0;
	} else if ((i + 3) % 7 == 0) {
		yvel = 60.0;
	} else {
		yvel = 90.0;
	}

	s = sin(phase + t*avel);
	c = cos(phase + t*avel);

	shape->pos[0] = fmod(range + t*xvel + range*sin(i), 2*range) - range;
	shape->pos[1] = -fmod(i*6.0 + t*yvel + range*sin(i), 2*range) + range;

	shape->m22[0] = scale * c; shape->m22[2] = scale * -s;
	shape->m22[1] = scale * s; shape->m22[3] = scale * c;
}

#define ALL_BUFFERS ( \
	GL_COLOR_BUFFER_BIT | \
	GL_DEPTH_BUFFER_BIT | \
	GL_STENCIL_BUFFER_BIT )

static int rain_(struct gl_api *api, struct gl_test *test)
{
	struct gl_core33 const *gl;
	struct xylo *xylo;
	struct xylo_glshape_set *set;
	struct xylo_glshape const *shape;
	struct xylo_dlist dlist;
	struct xylo_dshape dshapes[200];
	struct pfclock *clk;
	struct stopwatch sw;
	float const *color;
	double dt, et, et_mean, et_max, et_min;
	size_t i, n;
	GLuint gl_ns, query;
	struct xylo_view view;

	clk = pfclock_make();
	if (!clk) { return -1; }
	gl = gl_get_core33(api);
	if (!gl) { skip_test("OpenGL 3.3 or above required"); }
	xylo = make_xylo(api);
	if (!xylo) { return -1; }
	set = xylo_make_glshape_set(api, length_of(test_shape), test_shape);
	if (!set) { return -1; }

	/* create draw nodes */
	xylo_init_dlist(&dlist);
	for (i = 0; i < length_of(dshapes); i++) {
		shape = xylo_get_glshape(set, i % 4);
		color = i & 1 ? red : black;
		xylo_init_dshape(dshapes + i, color, shape);
		xylo_dlist_append(&dlist, &dshapes[i].draw);
	}

	gl->ClearColor(1.f, 1.f, 1.f, 1.f);
	gl->GenQueries(1, &query);
	n = 0;
	et = 0.0;
	et_mean = 0.0;
	et_min = HUGE_VAL;
	et_max = 0.0;

	stopwatch_start(&sw, pfclock_usec(clk));
	xylo_begin(xylo);
	xylo_set_shape_set(xylo, set);
	do {
		n++;
		dt = stopwatch_elapsed(&sw, pfclock_usec(clk)) * 1.e-6;
		for (i = 0; i < length_of(dshapes); i++) {
			update_transform(i, dt + 3.0, dshapes + i);
		}
		gl->BeginQuery(GL_TIME_ELAPSED, query);
		gl->Viewport(0, 0, gl_test_output_width, gl_test_output_height);
		gl->Clear(ALL_BUFFERS);
		update_view(gl, &view);
		xylo_draw(xylo, &view, &dlist.draw);
		gl->EndQuery(GL_TIME_ELAPSED);
		gl_test_swap_buffers(test);
		gl->Finish();
		gl->GetQueryObjectuiv(query, GL_QUERY_RESULT, &gl_ns);
		et = gl_ns * 1e-9;
		et_mean += et;
		if (et_max < et) { et_max = et; }
		if (et_min > et) { et_min = et; }
		if (et_mean > 1.0) {
			printf("%g\t%g\t%g\n", et_mean / n, et_max, et_min);
			et = 0.0;
			et_mean = 0.0;
			et_min = HUGE_VAL;
			et_max = 0.0;
			n = 0;
		}
	} while (gl_test_poll_key(test) == 0);
	xylo_end(xylo);

	xylo_term_dlist(&dlist);
	for (i = 0; i < length_of(dshapes); i++) {
		xylo_term_dshape(dshapes + i);
	}
	if (xylo_free_glshape_set(set, api)) { return -1; }
	free_xylo(xylo);
	pfclock_free(clk);
	return 0;
}
static int rain(void) { return run(rain_); }

struct test const tests[] = {
	{ creation, "create xylo renderer" },
	{ dlist, "render draw list" },
	{ transformed, "render items with tgraph" },
	{ rain, "one hundred shapes" },

	{ NULL, NULL }
};
