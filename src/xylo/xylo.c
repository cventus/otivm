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

#include "xylo.h"
#include "include/xylo.h"
#include "shapes.h"
#include "quincunx.h"

int init_xylo(struct xylo *xylo, struct gl_api *api)
{
	struct gl_core33 const *restrict gl;

	assert(api != NULL);
	assert(xylo != NULL);

	/* enforce minimum OpenGL version */
	if (gl = gl_get_core33(api), !gl) { return -1; }
	if (xylo_init_shapes(&xylo->shapes, api)) { return -2; }
	if (xylo_init_quincunx(&xylo->quincunx, api)) {
		xylo_term_shapes(&xylo->shapes, api);
		return -3;
	}
	xylo_init_fb(gl, &xylo->center_samples, 1);
	xylo_init_fb(gl, &xylo->corner_samples, 0);
	xylo->begin = 0;
	xylo->aa = 0;
	xylo->api = api;
	return 0;
}

struct xylo *make_xylo(struct gl_api *api)
{
	struct xylo *xylo = malloc(sizeof *xylo);
	if (xylo && init_xylo(xylo, api) != 0) {
		free(xylo);
		xylo = NULL;
	}
	return xylo;
}

void term_xylo(struct xylo *xylo)
{
	struct gl_core33 const *restrict gl;

	assert(xylo != NULL);
	gl = gl_get_core33(xylo->api);
	xylo_term_fb(gl, &xylo->center_samples);
	xylo_term_fb(gl, &xylo->corner_samples);
	xylo_term_shapes(&xylo->shapes, xylo->api);
	xylo_term_quincunx(&xylo->quincunx, xylo->api);
}

void free_xylo(struct xylo *xylo)
{
	term_xylo(xylo);
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
	if (!xylo->save.stencil_test) { gl->Enable(GL_STENCIL_TEST); }
	if (xylo->save.multisample) { gl->Disable(GL_MULTISAMPLE); }
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

unsigned xylo_get_object_id(struct xylo *xylo, GLsizei x, GLsizei y)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	return xylo_fb_object_id(gl, &xylo->center_samples, x, y);
}
