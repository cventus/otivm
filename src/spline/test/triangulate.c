#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "ok/ok.h"
#include "test.h"
#include "../triangulate.c"

static void check_delauney(
	char const *context,
	float const (*v)[2],
	size_t n,
	struct triangle_set *t)
{
	size_t i, j, a, b, c;
	float const *v0, *v1, *v2;
	void *connected_vertices;

	/* NOTE: does not check whether there are missing triangles */

	connected_vertices = calloc(bits_size(n), 1);
	if (!connected_vertices) {
		fail_test("%s: memory allocation failure\n", context);
	}
	for (i = 0; i < t->n; i++) {
		a = t->indices[i][0];
		b = t->indices[i][1];
		c = t->indices[i][2];
		v0 = v[a];
		v1 = v[b];
		v2 = v[c];

		bits_set(connected_vertices, a);
		bits_set(connected_vertices, b);
		bits_set(connected_vertices, c);

		/* triangle should be specified in counterclockwise order */
		if (!is_ccw(v0, v1, v2)) {
			ok = -1;
			printf("%s: triangle %zd is not counterclockwise\n",
			       context, i);
		}
		for (j = 0; j < n; j++) {
			if (j == a || j == b || j == c) continue;
			/* delauney property: no other point is within the
			   circle circumscribed on the three points of the
			   triangle */
			if (point2d_in_circle(v0, v1, v2, v[j])) {
				ok = -1;
				printf("%s: triangle %zd is not delauney\n",
				       context, i);
			}
		}
	}
	for (i = 0; i < n; i++) {
		/* assuming the points aren't all co-linear, every vertex
		   should be part of at least one triangle */
		if (!bits_get(connected_vertices, i)) {
			printf("%s: vertex %zd not part of any triangle\n",
			       context, i);
			ok = -1;
		}
	}

	for (i = 0; i < n; i++) {
		/* assuming the points aren't all co-linear, every vertex
		   should be part of at least one triangle */
		if (!bits_get(connected_vertices, i)) {
			printf("%s: vertex %zd not part of any triangle\n",
			       context, i);
			ok = -1;
		}
	}
	free(connected_vertices);
}

/* create a connected simple polygon defined by the counter-clockwise pts */
static eref simple_polygon(struct eset *set, float2 *pts, size_t n)
{
	float const (*p)[2];
	eref a, b, c;
	size_t i;

	if (eset_alloc(set, 1, &a)) {
nomem:		fail_test("memory allocation failed");
	}
	p = pts;
	*org(set, a) = p++;
	*dest(set, a) = p++;
	b = a;

	for (i = 2; i < n; i++) {
		if (eset_alloc(set, 1, &c)) { goto nomem; }
		*org(set, c) = *dest(set, b);
		*dest(set, c) = p;
		eset_splice(set, sym(b), c);
		b = c;
		p++;
	}
	if (eset_alloc(set, 1, &c)) { goto nomem; }
	eset_connect(set, b, a, c);
	return a;
}

int test_triangulate_points(void)
{
	size_t i;
	struct triangle_set *t;

	float const triangle[][2] = {
		{27.f, 10.f},
		{12.f, 53.f},
		{41.f, 39.f},
	};

	float const spades[][2] = {
		{ 0.02f,-0.24f},
		{-0.20f,-0.19f},
		{-0.02f,-0.08f},
		{-0.10f,-0.19f},
		{-0.02f,-0.24f},
		{ 0.00f,-0.32f},
		{ 0.40f, 0.20f},
		{-0.20f,-0.29f},
		{ 0.10f,-0.19f},
		{-0.40f, 0.00f},
		{ 0.02f,-0.08f},
		{ 0.20f,-0.29f},
		{ 0.20f,-0.19f},
		{ 0.00f, 0.55f},
		{-0.40f, 0.20f},
		{-0.40f,-0.19f},
		{ 0.00f, 0.50f},
		{ 0.40f,-0.19f},
		{ 0.20f, 0.35f},
		{ 0.40f, 0.00f},
		{-0.20f, 0.35f},
	};

#define EXAMPLE(name) { #name, name, length_of(name) }

	struct {
		char const *name;
		float const (*vertices)[2];
		size_t n;
	} examples[] = {
		EXAMPLE(triangle),
		EXAMPLE(spades)
	}, *e = examples;
	for (i = 0; i < length_of(examples); i++, e++) {
		t = triangle_set_triangulate(e->vertices, e->n, NULL, 0);
		if (!t) {
			ok = -1;
			printf("failed to triangulate %s\n", e->name);
			continue;
		}
		check_delauney(e->name, e->vertices, e->n, t);
		free(t);
	}
	return ok;
}

int test_triangulate_polygon(void)
{
	struct eset set;
	eref edge;
	size_t nvertex, nedge, i;
	int ntriangles;
	float2 *vertices;
	double pi, angle, half, sqrt2;
	struct triangle_set *triangle_set;

	pi = acos(0.0) * 2.0;
	angle = pi*2.0/5.0;
	half = angle / 2.0;
	sqrt2 = sqrt(2);

	float2 triangle[] = {
		{2.8f, 2.0f},
		{3.0f, 2.0f},
		{2.0f + sqrt2, 2.0f + sqrt2},
		{2.0f, 2.0f},
		{2.2f, 2.0f},
		{2.4f, 2.0f},
		{2.6f, 2.0f},
	}, pentagon[] = {
		{cos(pi/2.0 + angle*0), sin(pi/2.0 + angle*0)},
		{cos(pi/2.0 + angle*1), sin(pi/2.0 + angle*1)},
		{cos(pi/2.0 + angle*2), sin(pi/2.0 + angle*2)},
		{cos(pi/2.0 + angle*3), sin(pi/2.0 + angle*3)},
		{cos(pi/2.0 + angle*4), sin(pi/2.0 + angle*4)},
	}, star[] = {
		{2*cos(pi/2.0 + angle*0), 2*sin(pi/2.0 + angle*0)},
		{cos(pi/2.0 + angle*0 + half), sin(pi/2.0 + angle*0 + half)},
		{2*cos(pi/2.0 + angle*1), 2*sin(pi/2.0 + angle*1)},
		{cos(pi/2.0 + angle*1 + half), sin(pi/2.0 + angle*1 + half)},
		{2*cos(pi/2.0 + angle*2), 2*sin(pi/2.0 + angle*2)},
		{cos(pi/2.0 + angle*2 + half), sin(pi/2.0 + angle*2 + half)},
		{2*cos(pi/2.0 + angle*3), 2*sin(pi/2.0 + angle*3)},
		{cos(pi/2.0 + angle*3 + half), sin(pi/2.0 + angle*3 + half)},
		{2*cos(pi/2.0 + angle*4), 2*sin(pi/2.0 + angle*4)},
		{cos(pi/2.0 + angle*4 + half), sin(pi/2.0 + angle*4 + half)},
	}, concave_shape[] = {
		/* top right */
		{2.0f, 1.0f}, {2.0f, 2.0f}, {1.0f, 2.0f},

		/* top */
		{sqrt2 - 1, sqrt2}, {0.0f, 1.0f}, {1 - sqrt2, sqrt2},

		/* top left */
		{-1.0f, 2.0f}, {-2.0f, 2.0f}, {-2.0f, 1.0f},

		/* left */
		{-sqrt2, sqrt2 - 1}, {-1.0f, 0.0f}, {-sqrt2, 1 - sqrt2},

		/* bottom left */
		{-2.0f, -1.0f}, {-2.0f, -2.0f}, {-1.0f, -2.0f},

		/* bottom */
		{1 - sqrt2, -sqrt2}, {0.0f, -1.0f}, {sqrt2 - 1, -sqrt2},

		/* bottom right */
		{1.0f, -2.0f}, {2.0f, -2.0f}, {2.0f, -1.0f},

		/* right */
		{sqrt2, 1 - sqrt2}, {1.0f, 0.0f}, {sqrt2, sqrt2 - 1},
	};

	struct test_case {
		char const *name;
		float2 *vertices;
		size_t nvertex;
		size_t expected_triangles;
	} cases[] = {
		{
			.name = "triangle",
			.vertices = triangle,
			.nvertex = length_of(triangle),
			.expected_triangles = 5
		},
		{
			.name = "pentagon",
			.vertices = pentagon,
			.nvertex = length_of(pentagon),
			.expected_triangles = 3
		},
		{
			.name = "star",
			.vertices = star,
			.nvertex = length_of(star),
			.expected_triangles = 8
		},
		{
			.name = "concave shape",
			.vertices = concave_shape,
			.nvertex = length_of(concave_shape),
			.expected_triangles = 22
		},
	};

	init_eset(&set);
	for (i = 0; i < length_of(cases); i++) {
		vertices = cases[i].vertices;
		nvertex = cases[i].nvertex;
		edge = simple_polygon(&set, vertices, nvertex);
		ntriangles = triangulate_polygon(&set, edge);
		if (ntriangles < 0) {
			fail_test("%s: triangulation failed\n", cases[i].name);
		}
		nedge = ntriangles + nvertex;
		triangle_set = make_triangles(&set, edge, nedge, vertices,
		                               nvertex);
		if (triangle_set->n != cases[i].expected_triangles) {
			fail_test("%s: expected %zd triangles, got %zd\n",
			          cases[i].name,
			          cases[i].expected_triangles,
				  triangle_set->n);
		}
		free(triangle_set);
	}
	term_eset(&set);

	return ok;
}

int test_apply_constraints(void)
{
	enum { A, B, C, D, E, F, G, H, I, J };

	float2 points[] = {
		[A] = {1.0f, 1.0f},
		[B] = {0.5f, 7.0f},
		[C] = {2.0f, 4.0f},
		[D] = {5.0f, 3.0f},
		[E] = {5.0f, 6.0f},
		[F] = {7.0f, 2.0f},
		[G] = {8.0f, 8.0f},
		[H] = {8.0f, 9.0f},
		[I] = {9.0f, 5.0f},
		[J] = {9.5f, 8.0f},
	};
	float2 *spine[] = {
		points + A,
		points + B,
		points + C,
		points + D,
		points + E,
		points + F,
		points + G,
		points + H,
		points + I,
		points + J,
	};

	struct eset set[1];
	eref e, le, re, *emap;
	int edges;

	init_eset(set);

	edges = delauney(set, spine, length_of(points), &le, &re);
	if (edges < 0) { fail_test("delauney\n"); }
	emap = make_emap(set, le, points, length_of(points));
	if (!emap) { fail_test("malloc\n"); }
	if (add_constrained_edge(set, A, J, points, emap) != J) {
		fail_test("constrain\n");
	}
	e = emap[A];
	ok = -1;
	do {
		if (*dest(set, e) - points == J) { ok = 0; break; }
		e = onext(set, e);
	} while (e != emap[A]);
	if (ok) { fail_test("vertex A is not connected to J\n"); }
	ok = -1;
	e = emap[J];
	do {
		if (*dest(set, e) - points == A) { ok = 0; break; }
		e = onext(set, e);
	} while (e != emap[J]);
	if (ok) { fail_test("vertex J is not connected to A\n"); }

	free(emap);
	term_eset(set);

	return ok;
}
