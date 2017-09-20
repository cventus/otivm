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
#include "types.h"

#define GLSL(version,src) ("#version " #version " core\n" #src)

static struct gl_shader_source const vertex_shader = {
	GL_VERTEX_SHADER,
	GLSL(330,
	in vec2 shape_pos;
	in vec3 quadratic_pos;

	uniform mat4 mvp;

	out vertex_data
	{
		vec3 quadratic;
	} vertex;

	void main()
	{
		vertex.quadratic = quadratic_pos;
		gl_Position = mvp * vec4(shape_pos, 0.0, 1.0);
	})
};

static struct gl_shader_source const fragment_shader = {
	GL_FRAGMENT_SHADER,
	GLSL(330,

	uniform vec4 color;

	in vertex_data
	{
		vec3 quadratic;
	};

	out vec4 fill_color;

	void main()
	{
		float k = quadratic.x;
		float l = quadratic.y;
		float m = quadratic.z;

		if (k*k - l*m >= 0.0f) { discard; }
		fill_color = color;
	}),
};

enum
{
	FILL_COLOR_LOC = 0
};

static struct gl_location const shape_attributes[] = {
	{ SHAPE_POS_ATTRIB, "shape_pos" },
	{ QUADRATIC_POS_ATTRIB, "quadratic_pos" },
	{ 0, 0 }
};

#define UNIFORM(name) { offsetof(struct xylo_uniforms, name), #name }
static struct gl_uniform_layout const uniforms[] = {
	UNIFORM(mvp),
	UNIFORM(color),
	{ 0, 0 }
};
#undef UNIFORM

struct xylo *make_xylo(struct gl_api *api)
{
	struct xylo *xylo;
	struct gl_program_layout program = {
		(struct gl_shader_source[]) {
			vertex_shader,
			fragment_shader,
			{ GL_NONE, 0 }
		},
		shape_attributes,
		(struct gl_location[]){
			{ FILL_COLOR_LOC, "fill_color" },
			{ 0, 0 }
		}
	};
	struct gl_core33 const *restrict gl;

	gl = gl_get_core33(api);
	if (!gl) { return NULL; }
	xylo = malloc(sizeof *xylo);
	xylo->program = gl_make_program(api, &program);
	if (!xylo->program) {
		free(xylo);
		return NULL;
	}
	gl_get_uniforms(api, &xylo->uniforms, xylo->program, uniforms);
	xylo->api = api;
	return xylo;
}

void free_xylo(struct xylo *xylo)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	if (xylo->program) {
		gl_unuse_program(xylo->api, xylo->program);
		gl->DeleteProgram(xylo->program);
	}
	free(xylo);
}

void xylo_begin(struct xylo *xylo)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl->Enable(GL_STENCIL_TEST);
	gl->Disable(GL_MULTISAMPLE);
	gl->UseProgram(xylo->program);
}

void xylo_end(struct xylo *xylo)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl->Disable(GL_STENCIL_TEST);
	gl_unuse_program(xylo->api, xylo->program);
}

void xylo_set_shape_set(struct xylo *xylo, struct xylo_glshape_set *set)
{
	gl_get_core33(xylo->api)->BindVertexArray(set->vao);
}

void xylo_set_color4fv(struct xylo *xylo, float const *color)
{
	gl_get_core33(xylo->api)->Uniform4fv(xylo->uniforms.color, 1, color);
}

void xylo_set_mvp(struct xylo *xylo, float const *mvp)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl->UniformMatrix4fv(xylo->uniforms.mvp, 1, GL_FALSE, mvp);
}

static void draw_glshape(
	struct gl_core33 const *gl,
	struct xylo_glshape const *shape)
{
	gl->DrawElementsBaseVertex(
		GL_TRIANGLES,
		shape->count,
		shape->type,
		shape->indices,
		shape->basevertex);
}

void xylo_draw_glshape(struct xylo *xylo, struct xylo_glshape const *shape)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);

	/* Step one - Fill in stencil buffer*/
	gl->ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	gl->StencilFunc(GL_ALWAYS, 0x00, 0x01);
	gl->StencilOp(GL_KEEP, GL_KEEP, GL_INVERT);

	draw_glshape(gl, shape);

	/* Step two - fill in color without overdraw and erase stencil */
	gl->ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	gl->StencilFunc(GL_EQUAL, 0x01, 0x01);
	gl->StencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

	draw_glshape(gl, shape);
}
