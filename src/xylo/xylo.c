
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <base/mem.h>
#include <glapi/api.h>
#include <glapi/core.h>

#include "include/types.h"
#include "private.h"
#include "types.h"

struct xylo *make_xylo(struct gl_api *gl)
{
	struct xylo *xylo = malloc(sizeof *xylo);
	if (xylo_init_points_program(&xylo->points, gl)) { goto fail_points; }
	if (xylo_init_lines_program(&xylo->lines, gl)) { goto fail_lines; }
	if (xylo_init_spline_program(&xylo->spline, gl)) { goto fail_spline; }
	xylo->gl = gl;
	return xylo;

fail_spline:
	xylo_term_lines_program(&xylo->lines, gl);
fail_lines:
	xylo_term_points_program(&xylo->points, gl);
fail_points:
	free(xylo);
	return NULL;
}

void free_xylo(struct xylo *xylo)
{
	xylo_term_spline_program(&xylo->spline, xylo->gl);
	xylo_term_lines_program(&xylo->lines, xylo->gl);
	xylo_term_points_program(&xylo->points, xylo->gl);
	free(xylo);
}

void xylo_begin(struct xylo *xylo)
{
	struct gl_core const *restrict core = gl_get_core(xylo->gl);

	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_XOR);
	core->UseProgram(xylo->spline.name);
}

void xylo_end(struct xylo *xylo)
{
	struct gl_core const *restrict core = gl_get_core(xylo->gl);
	core->UseProgram(0);
	glDisable(GL_COLOR_LOGIC_OP);
}

void xylo_begin_points(struct xylo *xylo)
{
	glEnable(GL_PROGRAM_POINT_SIZE);
	gl_get_core(xylo->gl)->UseProgram(xylo->points.name);
}

void xylo_end_points(struct xylo *xylo)
{
	gl_get_core(xylo->gl)->UseProgram(0);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

void xylo_begin_lines(struct xylo *xylo)
{
	gl_get_core(xylo->gl)->UseProgram(xylo->lines.name);
}

void xylo_end_lines(struct xylo *xylo)
{
	gl_get_core(xylo->gl)->UseProgram(0);
}

void xylo_set_shape_set(struct xylo *xylo, struct xylo_glshape_set *set)
{
	struct gl_core const *restrict core = gl_get_core(xylo->gl);
	core->BindVertexArray(set->vao);
}

void xylo_draw_shape(struct xylo *xylo, struct xylo_glshape *shape)
{
	struct gl_core const *restrict core = gl_get_core(xylo->gl);
	core->MultiDrawArrays(
		GL_LINE_LOOP,
		shape->first,
		shape->count,
		shape->drawcount);
}

