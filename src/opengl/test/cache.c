
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

#include <ok/ok.h>
#include <rescache/rescache.h>

#include "../test.h"
#include "../types.h"
#include "../decl.h"

#define run(fn) gl_run_test(is_test_interactive() ? __func__ : NULL, fn)

static void white(void)
{
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
}

static int init_cache_(struct glstate *state, struct gltest *test)
{
	(void)state;
	white();
	gl_test_swap_buffers(test);
	return 0;
}
static int init_cache(void) { return run(init_cache_); }

static void check_shader(
	struct glstate *state,
	struct glshader *shader,
	char const *filename,
	GLint shader_type)
{
	GLint status;
	char *log;

	if (!shader) {
		printf("Shader is NULL\n");
		ok = -1;
		return;
	}
	if (!state->f.glIsShader(shader->name)) {
		printf("Not a shader: %s\n", filename);
		ok = -1;
		return;
	}
	if (gl_shader_type(state, shader) != shader_type) {
		printf("Invalid shader type\n");
		ok = -1;
	}
	state->f.glGetShaderiv(shader->name, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		ok = -1;
		log = gl_get_shader_info_log(state, shader);
		printf("shader log:\n%s\n", log);
		free(log);
	}
}

static int load_shader_(struct glstate *state, struct gltest *test)
{
	static char const *vs_filename =  "asset/test/shader.vert";
	static char const *fs_filename =  "asset/test/shader.frag";

	struct glshader *vert, *frag;
	//struct glprogram *program;

	vert = gl_load_vertex_shader(&state->cache, vs_filename);
	check_shader(state, vert, vs_filename, GL_VERTEX_SHADER);

	frag = gl_load_fragment_shader(&state->cache, fs_filename);
	check_shader(state, frag, fs_filename, GL_FRAGMENT_SHADER);

	white();

	gl_test_swap_buffers(test);

	if (vert) gl_release_vertex_shader(&state->cache, vert);
	if (frag) gl_release_fragment_shader(&state->cache, frag);

	return ok;
}
static int load_shader(void) { return run(load_shader_); }

static int load_material_(struct glstate *state, struct gltest *test)
{
	struct glgeometries const *const *geo;

	geo = gl_load_geometry(&state->cache, "asset/test/triangle.obj");
	if (!geo) { ok = -1; }

	gl_print_errors(__func__);

	white();

	gl_release_geometry(&state->cache, geo);

	gl_test_swap_buffers(test);

	return ok;
}
static int load_material(void) { return run(load_material_); }

struct test const tests[] = {
	{ init_cache, "Create the cache" },
	{ load_material, "Load material files" },
	{ load_shader, "Load shaders" },
	{ NULL, NULL }
};

