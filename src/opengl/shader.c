
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <fs/file.h>

#include "types.h"
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
	struct glstate *state,
	char const *source,
	GLuint shader)
{
	GLint compile_success;
	char *info_log;

	state->f.glShaderSource(shader, 1, (GLchar const **)&source, NULL);
	state->f.glCompileShader(shader);
	state->f.glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_success);
	if (compile_success) {
		return 0;
	} else {
		info_log = get_info_log(
			shader,
			state->f.glGetShaderiv,
			state->f.glGetShaderInfoLog);
		if (info_log) {
			fprintf(stderr, "glCompileShader():\n%s\n", info_log);
			free(info_log);
		}
		return -1;
	}
}

int gl_make_shader(
	struct glstate *state,
	struct glshader *shader,
	GLenum type,
	char const *source)
{
	GLuint name;

	name = state->f.glCreateShader(type);
	if (compile_shader(state, source, name)) {
		state->f.glDeleteShader(name);
		return -1;
	}
	shader->name = name;

	return 0;
}

void gl_free_shader(struct glstate *state, struct glshader const *shader)
{
	state->f.glDeleteShader(shader->name);
}



int gl_make_program(
	struct glstate *state,
	struct glprogram *program,
	struct glshader const *const *shaders,
	size_t nshaders)
{
	GLint link_success;
	GLuint name;
	size_t i;
	char *info_log;

	name = state->f.glCreateProgram();
	if (name == 0) { return -1; }
	program->name = name;
	for (i = 0; i < nshaders; i++) {
		state->f.glAttachShader(name, shaders[i]->name);
	}
	state->f.glLinkProgram(name);
	for(i = 0; i < nshaders; i++) {
		state->f.glDetachShader(name, shaders[i]->name);
	}
	state->f.glGetProgramiv(name, GL_LINK_STATUS, &link_success);
	if (link_success) {
		return 0;
	} else {
		info_log = gl_get_program_info_log(state, program);
		if (info_log) {
			fprintf(stderr, "glLinkProgram():\n%s\n", info_log);
			free(info_log);
		}
		state->f.glDeleteProgram(name);
		return -2;
	}
}

void gl_free_program(struct glstate *state, struct glprogram const *program)
{
	state->f.glDeleteProgram(program->name);
}

char *gl_get_shader_info_log(
	struct glstate *state,
	struct glshader const *shader)
{
	return get_info_log(
		shader->name,
		state->f.glGetShaderiv,
		state->f.glGetShaderInfoLog);
}

char *gl_get_program_info_log(
	struct glstate *state,
	struct glprogram const *program)
{
	return get_info_log(
		program->name,
		state->f.glGetProgramiv,
		state->f.glGetProgramInfoLog);
}

GLint gl_shader_type(
	struct glstate *state,
	struct glshader const *shader)
{
	GLint type;
	state->f.glGetShaderiv(shader->name, GL_SHADER_TYPE, &type);
	return type;
}

