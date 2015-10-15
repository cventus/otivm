
#include <stdio.h>
#include <ok/ok.h>
#include <ok/io.h>
#include <gm/misc.h>
#include <gm/vector.h>
#include "../include/wf.h"

static void chkidx(unsigned idx, size_t max, size_t group, size_t triangle,
                   size_t vertex, char const *part)
{
	if (idx < max) { return; }
	
	printf("group %zu, triangle %zu, vertex %zu: "
	       "%s=%u out of range [0, %zd)\n", group, triangle, vertex,
	       part, idx, max);
	ok = -1;
}

static void validate_indicies(struct wf_object const *obj)
{
	size_t i, j, k;
	struct wf_triangles const *group;
	const unsigned (*triangle)[3][3], (*vtx)[3];

	for (i = 0; i < obj->ngroups; i++) {
		group = obj->groups + i;
		for (j = 0; j < group->n; j++) {
			triangle = group->indicies + j;
			for (k = 0; k < 3; k++) {
				vtx = *triangle + k;
				chkidx((*vtx)[0], obj->npos, i, j, k, "pos");
				chkidx((*vtx)[1], obj->nuv, i, j, k, "uv");
				chkidx((*vtx)[2], obj->nnorm, i, j, k, "norm");
			}
		}
	}
}

static void validate_uv(struct wf_object const *obj)
{
	float const (*uv)[2];
	size_t i, j;

	uv = obj->uv;
	for (i = 0; i < obj->nuv; i++) {
		for (j = 0; j < 2; j++) {
			if (uv[i][j] < 0.f || uv[i][j] > 1.f) {
				printf("Texture coordinate %zu component `%s` "
				       "out of range [0, 1]: %g\n", i,
				       j == 0 ? "u" : "v", uv[i][j]);
				ok = -1;
			}
		}
	}
}

static void validate_norm(struct wf_object const *obj)
{
	float const (*norm)[3];
	size_t i;

	norm = obj->norm;
	for (i = 0; i < obj->nnorm; i++) {
		if (!fneareq(v3lenf(norm[i]), 1.f)) {
			printf("Invalid normal %zu: [%g %g %g]\n", i,
			       norm[i][0], norm[i][1], norm[i][2]);
		}
	}
}

static void validate_obj(struct wf_object const *obj)
{
	validate_indicies(obj);
	validate_uv(obj);
	validate_norm(obj);
}

static void assert_size(char const *msg, char const *name,
                        size_t value, size_t expected)
{
	if (value != expected) {
		printf("%s: %s is %zu, expected %zu\n", msg, name,
		       value, expected);
		ok = -1;
	}
}

static struct wf_object const *parse_string(char const *str)
{
	FILE *fp;
	struct wf_object const *obj;

	fp = open_str(str);
	obj = wf_fparse_object(fp);
	fclose(fp);
	return obj;
}

static void check_failure(char const *source, char const *msg)
{
	struct wf_object const *obj;
	obj = parse_string(source);
	if (obj) {
		wf_free_object(obj);
		printf("invalid input accepted: %s\n%s\n", msg, source);
	}
}

static void check_success(char const *source, char const *msg)
{
	struct wf_object const *obj;
	obj = parse_string(source);
	if (!obj) {
		printf("failed to parse: %s\n%s\n", msg, source);
	} else {
		wf_free_object(obj);
	}
}

static void assert_success(struct wf_object const *obj, char const *msg)
{
	if (!obj) {
		fail_test("failed to parse: %s\n", msg);
	}
}

#define assert_field(msg, o, field, expected) \
	assert_size(msg, #field, (o)->field, expected)

static int empty(void)
{
	struct { char const *contents, *message; } examples[] = {
		{ "", "empty file" },
		{ "\n\n", "newlines" },
		{ " \n\t \n ", "white space" },
		{ "# just a comment\n", "comment" },
		{
			"# This file is empty \n"
			"# except for comments \n"
			"# except for comments \n"
			"\n"
			"\n"
			"# EOF \n"
			"\n",
			"comments"
		}
	};

	struct wf_object const *obj;
	size_t i;

	for (i = 0; i < sizeof examples/ sizeof 0[examples]; i++) {
		obj = parse_string("");
		assert_success(obj, examples[i].message);

		assert_field(examples[i].message, obj, nmtllib, 0);
		assert_field(examples[i].message, obj, npos, 0);
		assert_field(examples[i].message, obj, nuv, 0);
		assert_field(examples[i].message, obj, nnorm, 0);
		assert_field(examples[i].message, obj, ngroups, 0);

		wf_free_object(obj);
	}

	return ok;
}

static int open_obj(void)
{
	struct wf_object const *obj;

	obj = parse_string(
		"# This defines a triangle \n"
		"v 1.5 3.0 .0\n"
		"v 0 0.0 .0\n"
		"v 3.0 0 .0\n"
		"\n"
		"# A few emtpy lines, and then the triangle\n"
		"\n"
		"f 1 2 3\n"
		"\n"
	);
	assert_success(obj, "simple file");
	validate_obj(obj);
	wf_free_object(obj);

	return ok;
}

static int incorrect(void)
{
	check_success("v  1  2.0  -3", "simple vertex");
	check_failure("v  c  2.0  -3", "non-numerical vertex component");
	check_failure("v  1  2.0abc4  -3", "non-numerical vertex component");
	check_failure("v  1  2.0  -k", "non-numerical vertex component");

	check_success("vt  0.1  0.2", "simple vertex");
	check_failure("vt -0.1  0.2", "negative texture coordinate");

	check_failure("v 1 2 3\n"
	              "v 2 3 1\n"
	              "f 1 3 2\n"
	              "v 3 1 2\n",
	              "invalid vertex index");

	check_failure("v 1 2 3\n"
	              "v 2 3 1\n"
	              "f -1 -3 -2\n"
	              "v 3 1 2\n",
	              "invalid vertex index");

	return ok;
}

static int parse_cube(void)
{
	struct wf_object const *obj;

	obj = parse_string(
		"# Vertices of a cube \n"
		"v -0.5 -0.5 -0.5\n"
		"v -0.5 -0.5  0.5\n"
		"v -0.5  0.5 -0.5\n"
		"v -0.5  0.5  0.5\n"
		"v  0.5 -0.5 -0.5\n"
		"v  0.5 -0.5  0.5\n"
		"v  0.5  0.5 -0.5\n"
		"v  0.5  0.5  0.5\n"
		"\n"

		"# Six normals for each cube face \n"
		"vn -1.0  0.0  0.0\n"
		"vn  1.0  0.0  0.0\n"
		"vn  0.0 -1.0  0.0\n"
		"vn  0.0  1.0  0.0\n"
		"vn  0.0  0.0 -1.0\n"
		"vn  0.0  0.0  1.0\n"
		"\n"

		"# Texture coordinates\n"
		"vt 0.0 1.0\n"
		"vt 1.0 1.0\n"
		"vt 0.0 0.0\n"
		"vt 1.0 0.0\n"
		"\n"

		"# Load a material library\n"
		"mtllib example.mtl\n"

		"# 8 faces (two triangles) for each side\n"
		"usemtl side\n"
		"f	1/1/1	2/2/1	3/3/1\n"
		"f	3/3/1	2/2/1	4/4/1\n"

		"usemtl side\n"
		"f	2/1/6	6/2/6	4/3/6\n"
		"f	4/3/6	6/2/6	8/4/6\n"

		"usemtl side\n"
		"f	6/1/2	5/2/2	8/3/2\n"
		"f	8/3/2	5/2/2	7/4/2\n"

		"usemtl side\n"
		"f	5/1/5	1/2/5	7/3/5\n"
		"f	7/3/5	1/2/5	3/4/5\n"

		"usemtl top\n"
		"f	7/1/4	3/2/4	8/3/4\n"
		"f	8/3/4	3/2/4	4/4/4\n"

		"usemtl bottom\n"
		"f	2/1/3	1/2/3	6/3/3\n"
		"f	6/3/3	1/2/3	5/4/3\n"

		"\n"
	);
	assert_success(obj, "cube");

	validate_obj(obj);
	assert_field("one loaded material", obj, nmtllib, 1);
	assert_field("eight vertices", obj, npos, 8);
	assert_field("four texture coordinates", obj, nuv, 4);
	assert_field("six normals", obj, nnorm, 6);
	assert_field("three material groups", obj, ngroups, 3);

	wf_free_object(obj);

	return ok;
}

struct test const tests[] = {
	{ empty,	"empty files" },
	{ open_obj,	"open and parse a simple file" },
	{ incorrect,	"incorrect inputs" },
	{ parse_cube,	"object with materials, vertices, normals, etc." },
	{ NULL, NULL }
};

