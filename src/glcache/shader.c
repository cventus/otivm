
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <fs/file.h>

#include <opengl/opengl.h>
#include <opengl/core.h>
#include "include/types.h"
#include "private.h"
#include "decl.h"
#include "shader.h"

static char *get_info_log(
	GLuint name,
	void (*get_iv)(GLuint, GLenum, GLint *),
	void (*info_log)(GLuint, GLsizei, GLsizei *, GLchar *))
{
	GLint len;
	char *p;

	get_iv(name, GL_INFO_LOG_LENGTH, &len);
	if (len <= 0) { return NULL; }
	p = malloc(len);
	if (p) { info_log(name, len, NULL, p); }
	return p;
}

static int compile_shader(
	struct gl_state *state,
	char const *source,
	GLuint shader)
{
	GLint compile_success;
	char *info_log;
	struct gl_core const *core;

	core = gl_get_core(state);
	core->ShaderSource(shader, 1, (GLchar const **)&source, NULL);
	core->CompileShader(shader);
	core->GetShaderiv(shader, GL_COMPILE_STATUS, &compile_success);
	if (compile_success) {
		return 0;
	} else {
		info_log = get_info_log(
			shader,
			core->GetShaderiv,
			core->GetShaderInfoLog);
		if (info_log) {
			fprintf(stderr, "glCompileShader():\n%s\n", info_log);
			free(info_log);
		}
		return -1;
	}
}

int gl_shader_init(
	struct gl_state *state,
	struct gl_shader *shader,
	GLenum type,
	char const *source)
{
	GLuint name;
	struct gl_core const *core = gl_get_core(state);

	name = core->CreateShader(type);
	if (compile_shader(state, source, name)) {
		core->DeleteShader(name);
		return -1;
	}
	shader->name = name;

	return 0;
}

void gl_shader_term(struct gl_state *state, struct gl_shader const *shader)
{
	struct gl_core const *core = gl_get_core(state);
	core->DeleteShader(shader->name);
}

int gl_program_init(
	struct gl_state *state,
	struct gl_program *program,
	struct gl_shader const *const *shaders,
	size_t nshaders)
{
	GLint link_success;
	GLuint name;
	size_t i;
	char *info_log;
	struct gl_core const *core;

	core = gl_get_core(state);
	name = core->CreateProgram();
	if (name == 0) { return -1; }
	program->name = name;
	for (i = 0; i < nshaders; i++) {
		core->AttachShader(name, shaders[i]->name);
	}
	core->LinkProgram(name);
	for(i = 0; i < nshaders; i++) {
		core->DetachShader(name, shaders[i]->name);
	}
	core->GetProgramiv(name, GL_LINK_STATUS, &link_success);
	if (link_success) {
		return 0;
	} else {
		info_log = gl_get_program_info_log(state, program);
		if (info_log) {
			fprintf(stderr, "glLinkProgram():\n%s\n", info_log);
			free(info_log);
		}
		core->DeleteProgram(name);
		return -2;
	}
}

void gl_program_term(struct gl_state *state, struct gl_program const *program)
{
	struct gl_core const *core = gl_get_core(state);
	core->DeleteProgram(program->name);
}

char *gl_get_shader_info_log(
	struct gl_state *state,
	struct gl_shader const *shader)
{
	struct gl_core const *core = gl_get_core(state);
	return get_info_log(
		shader->name,
		core->GetShaderiv,
		core->GetShaderInfoLog);
}

char *gl_get_program_info_log(
	struct gl_state *state,
	struct gl_program const *program)
{
	struct gl_core const *core = gl_get_core(state);
	return get_info_log(
		program->name,
		core->GetProgramiv,
		core->GetProgramInfoLog);
}

GLint gl_shader_type(
	struct gl_state *state,
	struct gl_shader const *shader)
{
	GLint type;
	struct gl_core const *core = gl_get_core(state);
	core->GetShaderiv(shader->name, GL_SHADER_TYPE, &type);
	return type;
}

