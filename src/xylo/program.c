
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <string.h>
#include <GL/gl.h>
#include <base/mem.h>
#include <glapi/api.h>
#include <glapi/core.h>

#include "include/types.h"
#include "private.h"
#include "types.h"

/* compile and link a set of shader sources */
GLuint xylo_make_program(struct gl_api *gl, struct program_def const *pdef)
{
	struct gl_core const *restrict core;
	struct shader const *p;
	struct program_location const *q;

	GLuint name, prog;
	GLsizei n;
	GLint status, len;

	assert(gl != NULL);
	assert(pdef != NULL);

	core = gl_get_core(gl);
	prog = core->CreateProgram();
	/* create and compile shaders */
	for (p = pdef->shaders; p->src; p++) {
		name = core->CreateShader(p->type);
		core->ShaderSource(name, 1, p->src, NULL);
		core->CompileShader(name);
		core->GetShaderiv(name, GL_COMPILE_STATUS, &status);
		if (status == 0) { goto clean; }
		core->AttachShader(prog, name);
	}
	/* bind attribute and fragment data locations */ 
	for (q = pdef->attrib; q->name; q++) {
		core->BindAttribLocation(prog, q->loc, q->name);
	}
	for (q = pdef->frag; q->name; q++) {
		core->BindFragDataLocation(prog, q->loc, q->name);
	}
	/* link */
	core->LinkProgram(prog);
	core->GetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status == 0) {
		core->GetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
		char *x = malloc(len + 1);
		core->GetProgramInfoLog(prog, len + 1, &len, x);
		printf("info log: %s\n", x);
	}
	/* clean up */
	while (core->GetAttachedShaders(prog, 1, &n, &name), n > 0) {
		core->DetachShader(prog, name);
clean:		core->DeleteShader(name);
	}
	if (status == 0) {
		core->DeleteProgram(prog);
		prog = 0;
	}
	return prog;
}

void xylo_program_uniforms(
	struct gl_api *gl,
	GLuint program,
	void *dest,
	struct program_uniform const *uniforms)
{
	struct gl_core const *restrict core;
	struct program_uniform const *p;
	GLuint *loc;

	core = gl_get_core(gl);
	for (p = uniforms; p->name; p++) {
		assert((p->offset % alignof(GLuint)) == 0);
		loc = (GLuint *)((char *)dest + p->offset);
		*loc = core->GetUniformLocation(program, p->name);
	}
}

