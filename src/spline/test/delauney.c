#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ok/ok.h>

#include "../delauney.c"

static void assert_eq(char const *as, char const *bs, eref a, eref b)
{
	if (a != b) {
		printf("not equal: %s = %ld != %ld = %s\n", as, (long)a, (long)b, bs);
		ok = -1;
	}
}
#define assert_eq(a, b) assert_eq(#a, #b, a, b)

static void assert_bool(char const *expr, int val, int target)
{
	if (!val != !target) {
		printf("not %s: %s \n", target ? "true" : "false",  expr);
		ok = -1;
	}
}
#define assert_true(x) assert_bool(#x, x, 1)
#define assert_false(x) assert_bool(#x, x, 0)

static void assert_near_eq(
	double a,
	char const *a_exp,
	double b,
	char const *b_exp)
{
	if (fabs(a - b) > 1e-6) {
		printf("%s = %g != %g = %s\n", a_exp, a, b, b_exp);
		ok = -1;
	}
}
#define assert_near_eq(a, b) assert_near_eq(a, #a, b, #b)

/* specify orbit with a sequence of edge references, ending in the first */
static void assert_orbit(
	struct eset *set,
	eref next(struct eset *, eref),
	char const *nextname,
	char const *args,
	eref first,
	...)
{
	va_list ap;
	struct { eref ref; char const *name; int len; } prev, cur;
	static char const *delim = ", \t\n";

	prev.ref = first;
	prev.name = args;
	prev.len = strcspn(prev.name, delim);
	va_start(ap, first);
	do {
		cur.ref = va_arg(ap, eref);
		cur.name = prev.name + prev.len;
		cur.name += strspn(cur.name, delim);
		cur.len = strcspn(cur.name, delim);
		if (next(set, prev.ref) != cur.ref) {
			printf("%.*s is not followed by %.*s, ",
				prev.len, prev.name,
				cur.len, cur.name);
			printf("%s(%.*s) = %d, expected %d\n",
				nextname,
				prev.len, prev.name,
				(int)next(set, prev.ref), (int)cur.ref);
			ok = -1;
			break;
		}
		memcpy(&prev, &cur, sizeof cur);
	} while (cur.ref != first);
	va_end(ap);
}
#define assert_orbit(set, next, first, ...) \
	assert_orbit(set, next, #next,  #first ", " #__VA_ARGS__, first, __VA_ARGS__)

static eref ref(eref e, unsigned r) {
	return mkref(e*4, r);
}

static struct edge edge(eref e, unsigned r) {
	return (struct edge){ 0, ref(e, r) };
}

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
			if (in_circle(v0, v1, v2, v[j])) {
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

	if (make_edges(set, 1, &a)) {
nomem:		fail_test("memory allocation failed");
	}
	p = pts;
	*org(set, a) = p++;
	*dest(set, a) = p++;
	b = a;

	for (i = 2; i < n; i++) {
		if (make_edges(set, 1, &c)) { goto nomem; }
		*org(set, c) = *dest(set, b);
		*dest(set, c) = p;
		splice(set, sym(b), c);
		b = c;
		p++;
	}
	if (make_edges(set, 1, &c)) { goto nomem; }
	connect(set, b, a, c);
	return a;
}

static int test_make_edge(void)
{
	struct eset set;
	eref e;

	init_eset(&set);

	if (make_edges(&set, 1, &e)) { fail_test("allocation failure"); }
	assert_eq(lnext(&set, e), sym(e));
	assert_eq(rnext(&set, e), sym(e));
	assert_eq(onext(&set, e), e);
	assert_eq(oprev(&set, e), e);

	e = rot(e);
	assert_eq(lnext(&set, e), e);
	assert_eq(rnext(&set, e), e);
	assert_eq(onext(&set, e), sym(e));
	assert_eq(oprev(&set, e), sym(e));

	term_eset(&set);

	return ok;
}

static int traverse(void)
{
	enum { a, b, c, d, e, f, g, h };

	/* Based on Fig. 7. from Guibas & Stolfi (1985) */
	struct edge edges[] = {
		[a*4] = edge(g, 3), edge(g, 2), edge(a, 2), edge(a, 1),
		[b*4] = edge(h, 3), edge(a, 0), edge(a, 3), edge(c, 2),
		[c*4] = edge(d, 3), edge(b, 0), edge(b, 3), edge(b, 2),
		[d*4] = edge(c, 3), edge(h, 0), edge(c, 1), edge(c, 0),
		[e*4] = edge(d, 0), edge(e, 3), edge(e, 2), edge(d, 1),
		[f*4] = edge(e, 0), edge(g, 1), edge(h, 1), edge(e, 1),
		[g*4] = edge(f, 2), edge(f, 1), edge(f, 0), edge(h, 2),
		[h*4] = edge(f, 3), edge(g, 0), edge(b, 1), edge(d, 2)
	};
	struct eset set = { { edges, edges + length_of(edges), NULL }, 0 };

	assert_eq(onext(&set, ref(a, 2)), ref(a, 2));

	assert_eq(onext(&set, ref(a, 0)), ref(g, 3));
	assert_eq(onext(&set, ref(g, 3)), ref(h, 2));
	assert_eq(onext(&set, ref(h, 2)), ref(b, 1));
	assert_eq(onext(&set, ref(b, 1)), ref(a, 0));

	assert_eq(onext(&set, ref(d, 1)), ref(h, 0));
	assert_eq(onext(&set, ref(h, 0)), ref(f, 3));
	assert_eq(onext(&set, ref(f, 3)), ref(e, 1));
	assert_eq(onext(&set, ref(e, 1)), sym(ref(e, 1)));
	assert_eq(onext(&set, sym(ref(e, 1))), ref(d, 1));

	return ok;
}

static int winding_order(void)
{
	typedef float xy[XY];

	xy A = { 0.0f, 0.0f };
	xy B = { 1.0f, 0.0f };
	xy C = { 0.0f, 1.0f };

	assert_true(is_ccw(A, B, C));
	assert_false(is_ccw(A, C, B));

	return ok;
}

static int test_in_circle(void)
{
	typedef float xy[XY];

	xy A = { 10.0f, 10.0f };
	xy B = { 11.0f, 10.0f };
	xy C = { 10.0f, 11.0f };

	xy D = { 10.1f, 10.1f };

	xy E = { 11.0f, 11.0f };
	xy F = { 12.0f, 12.0f };

	/* clearly inside */
	assert_true(in_circle(A, B, C, D));

	/* co-circular */
	assert_false(in_circle(A, B, C, E));

	/* clearly outside */
	assert_false(in_circle(A, B, C, F));

	return ok;
}

static int test_connect(void)
{
	struct eset set;
	eref a, b, c, d, e, f, g, h;

	/* Create the following subdivision with canonical edges going counter
	   clock wise around the edges:

	    O-------O
	   /|   g   |\
	 d/ |       | \b
	 /  |       |  \
	O   |f     c|   O
	 \  |       |  /
	 e\ |       | /a
	   \|   h   |/
	    O-------O
	*/

	init_eset(&set);

	if (make_edges(&set, 8, &a, &b, &c, &d, &e, &f, &g, &h)) {
		fail_test("allocation failure");
	}

	/* create triangle on the right */
	splice(&set, sym(a), b);
	connect(&set, b, a, c);

	assert_orbit(&set, onext, a, sym(c), a);
	assert_orbit(&set, onext, b, sym(a), b);
	assert_orbit(&set, onext, c, sym(b), c);
	assert_orbit(&set, lnext, a, b, c, a);
	assert_orbit(&set, rnext, sym(a), sym(b), sym(c), sym(a));

	/* create triangle on the left */
	splice(&set, sym(d), e);
	connect(&set, e, d, f);

	assert_orbit(&set, onext, d, sym(f), d);
	assert_orbit(&set, onext, e, sym(d), e);
	assert_orbit(&set, onext, f, sym(e), f);
	assert_orbit(&set, lnext, d, e, f, d);
	assert_orbit(&set, rnext, sym(d), sym(e), sym(f), sym(d));

	/* connect the two triangles forming a quad in the middle */
	connect(&set, sym(d), sym(b), g);
	connect(&set, sym(a), sym(e), h);

	assert_orbit(&set, onext, g, d, sym(f), g);
	assert_orbit(&set, onext, c, sym(b), sym(g), c);
	assert_orbit(&set, onext, h, a, sym(c), h);
	assert_orbit(&set, onext, f, sym(e), sym(h), f);

	/* inner quad orbit */
	assert_orbit(&set, rnext, h, c, g, f, h);

	/* outer shape orbit */
	assert_orbit(&set, lnext, h, sym(e), sym(d), g, sym(b), sym(a), h);

	term_eset(&set);

	return ok;
}

static int test_triangulate(void)
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
		t = triangulate(e->vertices, e->n);
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

static int signed_distance(void)
{
	/* ascending line y = x - 4 */
	float2 p0 = { 2.0f, -2.0f };
	float2 p1 = { 6.0f, 2.0f };

	/* points */
	float2 left_of = { 3.0f, 1.0f };
	float2 right_of = { 5.0f, -1.0f };

	/* expected distance */
	double abs_dist = sqrt(2.0);

#define dist(x) line_dist(make_line(p0, p1), x)
	assert_near_eq(dist(left_of), -abs_dist);
	assert_near_eq(dist(right_of), abs_dist);
#undef dist
	return ok;
}

static int test_triangulate_polygon(void)
{
	struct eset set;
	eref edge;
	size_t nvertex, nedge, i;
	int ntriangles;
	float2 *vertices;
	double pi, angle, half, r0, r1, sqrt2;
	struct triangle_set *triangle_set;

	pi = acos(0.0) * 2.0;
	angle = pi*2.0/5.0;
	r0 = 2;
	r1 = 5;
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
		{r1*cos(pi + angle*0), r1*sin(pi + angle*0)},
		{r0*cos(pi + angle*0 + half), r1*sin(pi + angle*0 + half)},
		{r1*cos(pi + angle*1), r1*sin(pi + angle*1)},
		{r0*cos(pi + angle*1 + half), r1*sin(pi + angle*1 + half)},
		{r1*cos(pi + angle*2), r1*sin(pi + angle*2)},
		{r0*cos(pi + angle*2 + half), r1*sin(pi + angle*2 + half)},
		{r1*cos(pi + angle*3), r1*sin(pi + angle*3)},
		{r0*cos(pi + angle*3 + half), r1*sin(pi + angle*3 + half)},
		{r1*cos(pi + angle*4), r1*sin(pi + angle*4)},
		{r0*cos(pi + angle*4 + half), r1*sin(pi + angle*4 + half)},
	}, concave_shape[] = {
		/* top right */
		{2.0f, 1.0f}, {2.0f, 2.0f}, {1.0f, 2.0f},

		/* top */
		{sqrt2, 1+sqrt2}, {0.0f, 1.0f}, {-sqrt2, 1+sqrt2},

		/* top left */
		{-1.0f, 2.0f}, {-2.0f, 2.0f}, {-2.0f, 1.0f},

		/* left */
		{-1-sqrt2, sqrt2}, {-1.0f, 0.0f}, {-1-sqrt2, -sqrt2},

		/* bottom left */
		{-2.0f, -1.0f}, {-2.0f, -2.0f}, {-1.0f, -2.0f},

		/* bottom */
		{-sqrt2, -1-sqrt2}, {0.0f, 1.0f}, {sqrt2, -1-sqrt2},

		/* bottom right */
		{1.0f, -2.0f}, {2.0f, -2.0f}, {2.0f, -1.0f},

		/* right */
		{1+sqrt2, -sqrt2}, {1.0f, 0.0f}, {1+sqrt2, sqrt2},
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
			          cases[i].expected_triangles,
				  triangle_set->n);
		}
		free(triangle_set);
	}
	term_eset(&set);

	return ok;
}

struct test const tests[] = {
	{ test_make_edge, "properties of new subdivision" },
	{ traverse, "traverse edges connected to a vertex/around a face" },
	{ winding_order, "winding order of points" },
	{ test_in_circle, "points inside and outside a circle" },
	{ test_connect, "connect points" },
	{ test_triangulate, "triangulate points" },
	{ signed_distance, "signed distance from point to line" },
	{ test_triangulate_polygon, "triangulate polygons" },

	{ NULL, NULL }
};
