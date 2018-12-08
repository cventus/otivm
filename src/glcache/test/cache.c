#include <stdio.h>
#include <stdlib.h>

#include "ok/ok.h"
#include "base/mem.h"
#include "rescache/rescache.h"
#include "glapi/api.h"
#include "glapi/core.h"
#include "glapi/test.h"
#include "glcache/cache.h"
#include "glcache/types.h"

#include "../private.h"
#include "../decl.h"
#include "../shader.h"

#define run(fn) gl_run_test(is_test_interactive() ? __func__ : NULL, fn)

static void white(struct gl_core30 const *gl)
{
	gl->ClearColor(1.f, 1.f, 1.f, 1.f);
	gl->Clear(GL_COLOR_BUFFER_BIT);
	gl->Flush();
}

static int init_cache_(struct gl_api *gl, struct gl_test *test)
{
	white(gl_get_core30(gl));
	gl_test_swap_buffers(test);
	if (is_test_interactive()) { gl_test_wait_for_key(test); }
	return 0;
}
static int init_cache(void) { return run(init_cache_); }

static void check_shader(
	struct gl_api *api,
	struct gl_shader const *shader,
	char const *filename,
	GLint shader_type)
{
	GLint status;
	char *log;
	struct gl_core30 const *gl = gl_get_core30(api);

	if (!shader) {
		printf("Shader is NULL\n");
		ok = -1;
		return;
	}
	if (!gl->IsShader(shader->name)) {
		printf("Not a shader: %s\n", filename);
		ok = -1;
		return;
	}
	if (gl_shader_type(api, shader) != shader_type) {
		printf("Invalid shader type\n");
		ok = -1;
	}
	gl->GetShaderiv(shader->name, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		ok = -1;
		log = gl_get_shader_info_log(api, shader);
		printf("shader log:\n%s\n", log);
		free(log);
	}
}

static int load_material_(struct gl_api *api, struct gl_test *test)
{
	struct gl_cache *cache;
	struct gl_geometries const *geo;
	int n;

	cache = gl_make_cache(api);
	geo = gl_load_geometry(cache, "asset/test/triangle.obj");
	if (!geo) { ok = -1; }

	white(gl_get_core30(api));

	gl_release_geometry(cache, geo);

	gl_test_swap_buffers(test);
	if (is_test_interactive()) { gl_test_wait_for_key(test); }
	if ((n = gl_free_cache(cache))) {
		printf("gl_free_cache returned %d\n", n);
	}

	return ok;
}
static int load_material(void) { return run(load_material_); }

static int load_shader_(struct gl_api *api, struct gl_test *test)
{
	int n;
	static char const *vs_filename =  "asset/test/shader.vert";
	static char const *fs_filename =  "asset/test/shader.frag";

	struct gl_shader const *vert, *frag;
	struct gl_cache *cache;

	cache = gl_make_cache(api);
	vert = gl_load_shader(cache, vs_filename);
	check_shader(api, vert, vs_filename, GL_VERTEX_SHADER);

	frag = gl_load_shader(cache, fs_filename);
	check_shader(api, frag, fs_filename, GL_FRAGMENT_SHADER);

	white(gl_get_core30(api));

	gl_test_swap_buffers(test);
	if (is_test_interactive()) { gl_test_wait_for_key(test); }

	if (vert) gl_release_shader(cache, vert);
	if (frag) gl_release_shader(cache, frag);
	if ((n = gl_free_cache(cache))) {
		printf("gl_free_cache returned %d\n", n);
	}

	return ok;
}
static int load_shader(void) { return run(load_shader_); }

static int load_program_(struct gl_api *api, struct gl_test *test)
{
	int n;
	char const *vs_filename =  "asset/test/shader.vert";
	char const *fs_filename =  "asset/test/shader.frag";

	char const *shaders1[] = { vs_filename, fs_filename };
	char const *shaders2[] = { fs_filename, vs_filename };

	struct gl_cache *cache;
	struct gl_program const *prog1, *prog2;

	cache = gl_make_cache(api);
	prog1 = gl_load_program(cache, shaders1, length_of(shaders1));

	if (!prog1) {
		fail_test("unable to open shader program!");
	}

	prog2 = gl_load_program(cache, shaders2, length_of(shaders2));
	if (prog2 != prog1) {
		ok = -1;
		printf("similar key yielded different programs\n");
	}
	if (prog2) gl_release_program(cache, prog2);

	white(gl_get_core30(api));

	gl_test_swap_buffers(test);
	if (is_test_interactive()) { gl_test_wait_for_key(test); }

	if (prog1) gl_release_program(cache, prog1);
	if ((n = gl_free_cache(cache))) {
		printf("gl_free_cache returned %d\n", n);
	}

	return ok;
}
static int load_program(void) { return run(load_program_); }

struct test const tests[] = {
	{ init_cache, "Create the cache" },
	{ load_material, "Load material files" },
	{ load_shader, "Load shaders" },
	{ load_program, "Load and link a program" },

	{ NULL, NULL }
};
