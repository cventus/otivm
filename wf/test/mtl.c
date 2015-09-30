
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#include <ok/ok.h>
#include <gm/misc.h>
#include <gm/vector.h>
#include <base/fwd.h>
#include <base/mem.h>

#include "../include/wf.h"
#include "../private.h"

#define material_field(name) { offsetof(struct wf_material, name), #name }
struct field
{
	size_t offset;
	char const *name;
};

static void assert_normalized(float value, char const *fmt, ...)
{
	va_list ap;

	if (value < 0.f || value > 1.f) {
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
		ok = -1;
	}
}

static void
validate_maps(char const *libname, struct wf_material const *mtl)
{
	static struct field const fields[] = {
		material_field(map_ka),
		material_field(map_kd),
		material_field(map_ks),
		material_field(map_ns),
		material_field(map_d),
		material_field(map_bump)
	};

	size_t i;
	struct wf_map *map;

	for (i = 0; i < length_of(fields); i++) {
		map = (struct wf_map *)((char *)mtl + fields[i].offset);
		if (map->filename == NULL) { continue; }
		if (strlen(map->filename) > 0) { continue; }
		printf("%s.%s.%s.filename of zero length!\n", libname,
		       mtl->name, fields[i].name);
		ok = -1;
	}
}

static void
validate_colors(char const *libname, struct wf_material const *mtl)
{
	static struct field const fields[] = {
		material_field(ka), material_field(kd), material_field(ks)
	};

	size_t i, j;
	float (*col)[3], val;

	for (i = 0; i < length_of(fields); i++) {
		col = (float (*)[3])((char *)mtl + fields[i].offset);
		for (j = 0; j < length_of(*col); j++) {
			val = (*col)[j];
			assert_normalized(val,
				"%s.%s.%s[%zu] out of range: %g\n",
				libname, mtl->name, fields[i].name, j, val);
		}
	}

	assert_normalized(mtl->d, "%s.%s: dissolve out of range: %g\n",
				libname, mtl->name, val);
}

static void
validate_mtllib(char const *libname, struct wf_mtllib const *mtllib)
{
	size_t i, j;

	for (i = 0; i < mtllib->n; i++) {
		/* Validate name */
		if (!mtllib->m[i].name) {
			/* A missing name indicates a parsing error */
			printf("%s: unnamed material %zu\n", libname, i);
			ok = -1;
		} else if (!mtllib->m[i].name[0]) {
			/* Object files cannot refer to materials with an
			   empty name, and is regarded as a parsing error. */
			printf("%s: empty material name %zu\n", libname, i);
			ok = -1;
		} else {
			/* Two materials in a library cannot have the same
			   name. */
			for (j = i + 1; j < mtllib->n; j++) {
				if (streq(mtllib->m[i].name, 
				          mtllib->m[j].name)) {
					printf("%s: materials %zu and %zu "
					       "have the same name ``%s''\n",
					       libname, i, j, mtllib->m[i].name);
					ok = -1;
				}
			}
		}
		/* Validate fields */
		validate_colors(libname, mtllib->m + i);
		validate_maps(libname, mtllib->m + i);
	}
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

static struct wf_mtllib const *parse_string(char const *str)
{
	FILE *fp;
	struct wf_mtllib const *mtllib;

	fp = open_str(str);
	mtllib = wf_fparse_mtllib(fp);
	fclose(fp);
	return mtllib;
}

static void check_failure(char const *source, char const *msg)
{
	struct wf_mtllib const *mtllib;
	mtllib = parse_string(source);
	if (mtllib) {
		wf_free_mtllib(mtllib);
		printf("invalid input accepted: %s\n%s\n", msg, source);
	}
}

static void check_success(char const *source, char const *msg)
{
	struct wf_mtllib const *mtllib;
	mtllib = parse_string(source);
	if (!mtllib) {
		printf("failed to parse: %s\n%s\n", msg, source);
	} else {
		wf_free_mtllib(mtllib);
	}
}

static void assert_success(struct wf_mtllib const *mtllib, char const *msg)
{
	if (!mtllib) { fail_test("failed to parse: %s\n", msg); }
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

	struct wf_mtllib const *mtllib;
	size_t i;

	for (i = 0; i < sizeof examples/ sizeof 0[examples]; i++) {
		mtllib = parse_string(examples[i].contents);
		assert_success(mtllib, examples[i].message);
		assert_field(examples[i].message, mtllib, n, 0);
		wf_free_mtllib(mtllib);
	}

	return ok;
}

static int open_mtllib(void)
{
	struct wf_mtllib const *mtllib;

	mtllib = parse_string(
		"# Material ``my_mtl'' \n"
 		"newmtl my_mtl\n"
 		"\n"
		"# Colors\n"
 		"Ka 0.0435 0.0435 0.0435\n"
 		"Kd 0.1086 0.1086 0.1086\n"
 		"Ks 0.0000 0.0000 0.0000\n"
 		"illum 6\n"
 		"d 0.6600\n"
 		"Ns 10.0000\n"
		"\n"
		"# Texture maps\n"
 		"map_Ka example.tm\n"
 		"map_Kd example.tm\n"
 		"map_Ks example.tm\n"
 		"map_Ns example-exp.sm\n"
 		"map_d example-dif.sm\n"
 		"bump example-bump.sm\n"
	);
	assert_success(mtllib, "single");
	validate_mtllib("single", mtllib);
	wf_free_mtllib(mtllib);

	return ok;
}

#define PREFIX "newmtl my_mtl\n"

static int colors(void)
{
	check_success(PREFIX "Ka 0.5 0.5 0.5", "Simple ambient color");
	check_failure(PREFIX "Ka a 0.5 0.5", "non-numerical color component");
	check_failure(PREFIX "Ka 0.5 -0.5 0.5", "negative color component");
	check_failure(PREFIX "Ka 0.5 0.5 1.5", "out-of-range color component");

	return ok;
}

static int parse_many(void)
{
	struct wf_mtllib const *mtllib;

	mtllib = parse_string(
		"newmtl one\n"
		"Ka 0 0.5 1\n"
		"map_Kd my-texture.tm\n"
		"\n"
		"newmtl two\n"
 		"Ka 0.0435 0.0435 0.0435\n"
 		"Kd 0.1086 0.1086 0.1086\n"
 		"Ks 0.0000 0.0000 0.0000\n"
 		"illum 6\n"
 		"d 0.6600\n"
 		"Ns 10.0000\n"
 		"map_Ka chrome.mpc\n"
 		"map_Kd chrome.mpc\n"
 		"map_Ks chrome.mpc\n"
 		"map_Ns wisp.mps\n"
 		"map_d wisp.mps\n"
 		"bump sand.mpb\n"
		"\n"
		"newmtl three\n"
		"Ka 0.0000 0.0000 0.0000\n"
		"Kd 0.0000 0.0000 0.0000\n"
		"Ks 0.6180 0.8760 0.1430\n"
		"Ns 200\n"
);
	assert_success(mtllib, "many");

	assert_field("three materials", mtllib, n, 3);
	validate_mtllib("many", mtllib);

	wf_free_mtllib(mtllib);

	return ok;
}

struct test const tests[] = {
	{ empty,	"empty files" },
	{ open_mtllib,	"open and parse a material" },
	{ colors,	"color validation" },
	{ parse_many,	"several plausible materials in one file" },
	{ NULL, NULL }
};

