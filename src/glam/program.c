#include <stdlib.h>

#include <base/mem.h>
#include <glapi/api.h>
#include <glapi/core.h>

#include "include/program.h"

GLuint gl_make_program(struct gl_api *api, struct gl_program_layout const *pl)
{
	struct gl_core30 const *restrict gl;
	GLchar const *src;
	GLuint shader, prog, result, names[16];
	GLint status;
	GLsizei count;
	size_t i;
	struct gl_shader_source const *source;
	struct gl_location const *attrib, *frag;

	if (!api || !pl) { return 0; }
	gl = gl_get_core30(api);
	result = prog = gl->CreateProgram();
	for (attrib = pl->attrib; attrib && attrib->name; attrib++) {
		gl->BindAttribLocation(prog, attrib->index, attrib->name);
	}
	for (frag = pl->frag; frag && frag->name; frag++) {
		gl->BindFragDataLocation(prog, frag->index, frag->name);
	}
	for (source = pl->source; source && source->source; source++) {
		shader = gl->CreateShader(source->type);
		src = source->source;
		gl->ShaderSource(shader, 1, &src, NULL);
		gl->CompileShader(shader);
		gl->GetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == 0) {
			gl->DeleteShader(shader);
			result = 0;
			goto clean;
		}
		gl->AttachShader(prog, shader);
	}
	gl->LinkProgram(prog);
	gl->GetProgramiv(prog, GL_LINK_STATUS, &status);
clean:	do {
		gl->GetAttachedShaders(prog, length_of(names), &count, names);
		for (i = count; i --> 0; ) {
			gl->DetachShader(prog, names[i]);
			gl->DeleteShader(names[i]);
		}
	} while (count > 0);
	if (status == 0) {
		gl->DeleteProgram(prog);
	}
	return result;
}

void gl_get_uniforms(
        struct gl_api *api,
        void *dest,
        GLuint program,
        struct gl_uniform_layout const *uniforms)
{
        struct gl_uniform_layout const *u;
        GLint loc, (*GetUniformLocation)(GLuint, GLchar const *);

        GetUniformLocation = gl_get_core30(api)->GetUniformLocation;
	for (u = uniforms; u->name; u++) {
                loc = GetUniformLocation(program, u->name);
                if (loc < 0) { loc = GL_INVALID_INDEX; }
                *(GLuint *)((char *)dest + u->offset) = loc;
        }
}

void gl_unuse_program(struct gl_api *api, GLuint name)
{
        GLint current;
	struct gl_core30 const *restrict gl;

        gl = gl_get_core30(api);
	gl->GetIntegerv(GL_CURRENT_PROGRAM, &current);
        if (name == (GLuint)current) {
                gl->UseProgram(0);
       }
}
