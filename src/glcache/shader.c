#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fs/file.h>

#include <glapi/api.h>
#include <glapi/core.h>
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
	struct gl_api *api,
	char const *source,
	GLuint shader)
{
	GLint compile_success;
	char *info_log;
	struct gl_core30 const *restrict gl;

	gl = gl_get_core30(api);
	gl->ShaderSource(shader, 1, (GLchar const **)&source, NULL);
	gl->CompileShader(shader);
	gl->GetShaderiv(shader, GL_COMPILE_STATUS, &compile_success);
	if (compile_success) {
		return 0;
	} else {
		info_log = get_info_log(
			shader,
			gl->GetShaderiv,
			gl->GetShaderInfoLog);
		if (info_log) {
			fprintf(stderr, "glCompileShader():\n%s\n", info_log);
			free(info_log);
		}
		return -1;
	}
}

int gl_shader_init(
	struct gl_api *api,
	struct gl_shader *shader,
	GLenum type,
	char const *source)
{
	GLuint name;
	struct gl_core30 const *restrict gl = gl_get_core30(api);

	name = gl->CreateShader(type);
	if (compile_shader(api, source, name)) {
		gl->DeleteShader(name);
		return -1;
	}
	shader->name = name;

	return 0;
}

void gl_shader_term(struct gl_api *api, struct gl_shader const *shader)
{
	struct gl_core30 const *restrict gl = gl_get_core30(api);
	gl->DeleteShader(shader->name);
}

int gl_program_init(
	struct gl_api *api,
	struct gl_program *program,
	struct gl_shader const *const *shaders,
	size_t nshaders)
{
	GLint link_success;
	GLuint name;
	size_t i;
	char *info_log;
	struct gl_core30 const *restrict gl;

	gl = gl_get_core30(api);
	name = gl->CreateProgram();
	if (name == 0) { return -1; }
	program->name = name;
	for (i = 0; i < nshaders; i++) {
		gl->AttachShader(name, shaders[i]->name);
	}
	gl->LinkProgram(name);
	for(i = 0; i < nshaders; i++) {
		gl->DetachShader(name, shaders[i]->name);
	}
	gl->GetProgramiv(name, GL_LINK_STATUS, &link_success);
	if (link_success) {
		return 0;
	} else {
		info_log = gl_get_program_info_log(api, program);
		if (info_log) {
			fprintf(stderr, "glLinkProgram():\n%s\n", info_log);
			free(info_log);
		}
		gl->DeleteProgram(name);
		return -2;
	}
}

void gl_program_term(struct gl_api *api, struct gl_program const *program)
{
	struct gl_core30 const *restrict gl = gl_get_core30(api);
	gl->DeleteProgram(program->name);
}

char *gl_get_shader_info_log(
	struct gl_api *api,
	struct gl_shader const *shader)
{
	struct gl_core30 const *gl = gl_get_core30(api);
	return get_info_log(
		shader->name,
		gl->GetShaderiv,
		gl->GetShaderInfoLog);
}

char *gl_get_program_info_log(
	struct gl_api *api,
	struct gl_program const *program)
{
	struct gl_core30 const *gl = gl_get_core30(api);
	return get_info_log(
		program->name,
		gl->GetProgramiv,
		gl->GetProgramInfoLog);
}

GLint gl_shader_type(
	struct gl_api *api,
	struct gl_shader const *shader)
{
	GLint type;
	struct gl_core30 const *restrict gl = gl_get_core30(api);
	gl->GetShaderiv(shader->name, GL_SHADER_TYPE, &type);
	return type;
}
