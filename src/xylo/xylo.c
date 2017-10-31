#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <string.h>
#include <base/mem.h>

#include <glapi/api.h>
#include <glapi/core.h>
#include <glam/program.h>
#include <gm/matrix.h>

#include "include/types.h"
#include "include/xylo.h"
#include "private.h"
#include "fb.h"
#include "types.h"

struct xylo *make_xylo(struct gl_api *api)
{
	struct xylo *xylo;
	struct gl_core33 const *restrict gl;

	gl = gl_get_core33(api);
	if (!gl) { return NULL; }
	xylo = malloc(sizeof *xylo);
	if (xylo_init_shapes(&xylo->shapes, api)) {
		free(xylo);
		return NULL;
	}
	if (xylo_init_quincunx(&xylo->quincunx, api)) {
		gl->DeleteProgram(xylo->shapes.program);
		free(xylo);
		return NULL;
	}
	xylo_init_fb(gl, &xylo->center_samples, 1);
	xylo_init_fb(gl, &xylo->corner_samples, 0);
	xylo->begin = 0;
	xylo->api = api;
	return xylo;
}

void free_xylo(struct xylo *xylo)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl_unuse_program(xylo->api, xylo->shapes.program);
	gl_unuse_program(xylo->api, xylo->quincunx.program);
	gl->DeleteProgram(xylo->shapes.program);
	xylo_term_quincunx(&xylo->quincunx, gl);
	free(xylo);
}

void xylo_begin(struct xylo *xylo)
{
	struct gl_core33 const *restrict gl;

	if (xylo->begin++ > 0) { return; }
	gl = gl_get_core33(xylo->api);

	/* save GL state */
	xylo->save.stencil_test = gl->IsEnabled(GL_STENCIL_TEST);
	xylo->save.multisample = gl->IsEnabled(GL_MULTISAMPLE);
	gl->Enable(GL_STENCIL_TEST);
	gl->Disable(GL_MULTISAMPLE);
}

void xylo_end(struct xylo *xylo)
{
	struct gl_core33 const *restrict gl;

	if (--xylo->begin > 0) { return; }
	gl = gl_get_core33(xylo->api);

	/* restore GL state */
	if (!xylo->save.stencil_test) { gl->Disable(GL_STENCIL_TEST); }
	if (xylo->save.multisample) { gl->Enable(GL_MULTISAMPLE); }
}

void xylo_set_shape_set(struct xylo *xylo, struct xylo_glshape_set *set)
{
	gl_get_core33(xylo->api)->BindVertexArray(set->vao);
}

GLuint xylo_get_uint(struct gl_core33 const *restrict gl, GLenum t)
{
	GLint v;
	gl->GetIntegerv(t, &v);
	return (GLuint)v;
}

unsigned xylo_get_object_id(
	struct xylo *xylo,
	GLsizei x,
	GLsizei y)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	return xylo_fb_object_id(gl, &xylo->center_samples, x, y);
}
