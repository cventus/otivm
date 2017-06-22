#include <stdlib.h>
#include <GL/gl.h>

#include <base/mem.h>
#include <glapi/api.h>
#include <glapi/core.h>

#include "include/program.h"

GLuint gl_make_program(struct gl_api *gl, struct gl_program_layout const *pl)
{
	struct gl_core const *restrict core;
	GLchar const *src;
	GLuint shader, prog, result, names[16];
	GLint status;
	GLsizei count;
	size_t i;
	struct gl_shader_source const *source;
	struct gl_location const *attrib, *frag;

	if (!gl || !pl) { return 0; }
	core = gl_get_core(gl);
	result = prog = core->CreateProgram();
	for (attrib = pl->attrib; attrib && attrib->name; attrib++) {
		core->BindAttribLocation(prog, attrib->index, attrib->name);
	}
	for (frag = pl->frag; frag && frag->name; frag++) {
		core->BindFragDataLocation(prog, frag->index, frag->name);
	}
	for (source = pl->source; source && source->source; source++) {
		shader = core->CreateShader(source->type);
		src = source->source;
		core->ShaderSource(shader, 1, &src, NULL);
		core->CompileShader(shader);
		core->GetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == 0) {
			core->DeleteShader(shader);
			result = 0;
			goto clean;
		}
		core->AttachShader(prog, shader);
	}
	core->LinkProgram(prog);
	core->GetProgramiv(prog, GL_LINK_STATUS, &status);
clean:	do {
		core->GetAttachedShaders(prog, length_of(names), &count, names);
		for (i = count; i --> 0; ) {
			core->DetachShader(prog, names[i]);
			core->DeleteShader(names[i]);
		}
	} while (count > 0);
	if (status == 0) {
		core->DeleteProgram(prog);
	}
	return result;
}

void gl_get_uniforms(
        struct gl_api *gl,
        void *dest,
        GLuint program,
        struct gl_uniform_layout const *uniforms)
{
        struct gl_uniform_layout const *u;
        GLint loc, (*GetUniformLocation)(GLuint, GLchar const *);

        GetUniformLocation = gl_get_core(gl)->GetUniformLocation;
	for (u = uniforms; u->name; u++) {
                loc = GetUniformLocation(program, u->name);
                if (loc < 0) { loc = GL_INVALID_INDEX; }
                *(GLuint *)((char *)dest + u->offset) = loc;
        }
}

void gl_unuse_program(struct gl_api *gl, GLuint name)
{
        GLint current;
        gl_get_core(gl)->GetIntegerv(GL_CURRENT_PROGRAM, &current);
        if (name == (GLuint)current) {
                gl_get_core(gl)->UseProgram(0);
       }
}
