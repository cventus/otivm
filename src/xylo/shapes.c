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

	/* custom multisampling support for 2 or 4 samples */
	uniform vec2 sample_clip[4];
	uniform vec4 sample_offset[4];

	in vec2 shape_pos;
	in vec3 quadratic_pos;

	out vec3 quadratic;
	out gl_PerVertex
	{
		vec4 gl_Position;
		float gl_ClipDistance[2];
	};

	void main()
	{
		quadratic = quadratic_pos;
		gl_Position = mvp * vec4(shape_pos, 0.0, 1.0) +
			sample_offset[gl_InstanceID];

		/* clip-space clip-planes through center defined by axis
		   aligned normal directions */
		gl_ClipDistance[0] = sample_clip[gl_InstanceID].x * gl_Position.x;
		gl_ClipDistance[1] = sample_clip[gl_InstanceID].y * gl_Position.y;
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
	UNIFORM(sample_clip),
	UNIFORM(sample_offset),
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

void xylo_shapes_set_sample_clip(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	GLsizei n,
	float const *sample_clip)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == shapes->program);
	gl->Uniform2fv(shapes->sample_clip, n, sample_clip);
}

void xylo_shapes_set_sample_offset(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	GLsizei n,
	float const *sample_offset)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == shapes->program);
	gl->Uniform4fv(shapes->sample_offset, n, sample_offset);
}

void xylo_shapes_set_object_id(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	unsigned object_id)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == shapes->program);
	gl->Uniform1ui(shapes->object_id, object_id);
}

void xylo_shapes_set_sample_offsets(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	unsigned object_id)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == shapes->program);
	gl->Uniform1ui(shapes->object_id, object_id);
}
