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
#include "xylo.h"
#include "shapes.h"
#include "private.h"
#include "types.h"

static struct gl_shader_source const shapes_vert = {
	GL_VERTEX_SHADER,
	GLSL(330,
	uniform mat4 mvp;

	in vec2 shape_pos;
	in vec3 quadratic_pos;

	out vec3 quadratic;

	void main()
	{
		quadratic = quadratic_pos;
		gl_Position = mvp * vec4(shape_pos, 0.0, 1.0);
	})
};

static struct gl_shader_source const shapes_frag = {
	GL_FRAGMENT_SHADER,
	GLSL(330,

	uniform vec4 color;
	uniform uint object_id;

	in vec3 quadratic;

	out vec4 fill_color;
	out uint fragment_id;

	void main()
	{
		float k = quadratic.x;
		float l = quadratic.y;
		float m = quadratic.z;

		if (k*k - l*m >= 0.0f) { discard; }
		fill_color = color;
		fragment_id = object_id;
	})
};

static struct gl_location const fragment_locs[] = {
	{ FILL_COLOR_LOC, "fill_color" },
	{ FRAGMENT_ID_LOC, "fragment_id" },
	{ 0, 0 }
};

static struct gl_location const shapes_attrib[] = {
	{ ATTRIB_SHAPE_POS, "shape_pos" },
	{ ATTRIB_QUADRATIC_POS, "quadratic_pos" },
	{ 0, 0 }
};

#define UNIFORM(name) { offsetof(struct xylo_shapes, name), #name }
static struct gl_uniform_layout const uniforms[] = {
	UNIFORM(mvp),
	UNIFORM(color),
	UNIFORM(object_id),
	{ 0, 0 }
};
#undef UNIFORM

int xylo_init_shapes(struct xylo_shapes *shapes, struct gl_api *api)
{
	struct gl_program_layout program = {
		(struct gl_shader_source[]) {
			shapes_vert,
			shapes_frag,
			{ GL_NONE, 0 }
		},
		shapes_attrib,
		fragment_locs
	};
	shapes->program = gl_make_program(api, &program);
	if (!shapes->program) { return -1; }
	gl_get_uniforms(api, shapes, shapes->program, uniforms);
	return 0;
}

void xylo_term_shapes(struct xylo_shapes *shapes, struct gl_api *api)
{
	gl_get_core30(api)->DeleteProgram(shapes->program);
	gl_unuse_program(api, shapes->program);
}

void xylo_shapes_set_color4fv(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	float const *color)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == shapes->program);
	gl->Uniform4fv(shapes->color, 1, color);
}

void xylo_shapes_set_mvp(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	float const *mvp)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == shapes->program);
	gl->UniformMatrix4fv(shapes->mvp, 1, GL_FALSE, mvp);
}

void xylo_shapes_set_object_id(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	unsigned object_id)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == shapes->program);
	gl->Uniform1ui(shapes->object_id, object_id);
}
