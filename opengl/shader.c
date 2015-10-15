
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <fs/file.h>

#include "types.h"
#include "decl.h"

static int compile_shader(
	struct glstate *state,
	char const *filename,
	GLuint shader)
{
	FILE *fp;
	char *data;

	fp = fopen(filename, "r");
	if (!fp) { return -1; }
	data = read_all(fp);
	if (!data) { return -2; }
	state->f.glShaderSource(shader, 1, (GLchar const **)&data, NULL);
	state->f.glCompileShader(shader);
	free(data);
	return 0;
}

int gl_make_shader(
	struct glstate *state,
	char const *filename,
	GLenum type,
	struct glshader *shader)
{
	GLuint name;

	name = state->f.glCreateShader(type);
	if (compile_shader(state, filename, name)) {
		state->f.glDeleteShader(name);
		return -1;
	}
	shader->name = name;

	return 0;
}

char *gl_get_shader_info_log(struct glstate *state, struct glshader *shader)
{
	GLint len;
	char *p;

	state->f.glGetShaderiv(shader->name, GL_INFO_LOG_LENGTH, &len);
	if (len <= 0) { return NULL; }
	p = malloc(len);
	state->f.glGetShaderInfoLog(shader->name, len, NULL, p);
	return p;
}

GLint gl_shader_type(struct glstate *state, struct glshader *shader)
{
	GLint type;
	state->f.glGetShaderiv(shader->name, GL_SHADER_TYPE, &type);
	return type;
}

