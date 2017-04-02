
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
#include "attributes.h"

static char const *const vs_src = GLSL(330,

in vec2 model_lpos;
in vec2 model_wpos;

uniform mat4 to_world;
uniform mat4 to_clip;

out vertex_layout
{
	vec4 clip_lpos;
	vec4 clip_wpos;
} vertex;

void main()
{
	mat4 m = to_clip * to_world;
	vertex.clip_lpos = m * vec4(model_lpos, 0.0, 1.0);
	vertex.clip_wpos = m * vec4(model_wpos, 0.0, 1.0);
}
);

static char const *const gs_src = GLSL(330,

layout(lines) in;
layout(points, max_vertices = 2) out;

uniform vec4 ctrl_color_size;
uniform vec4 knot_color_size;

in vertex_layout
{
	vec4 clip_lpos;
	vec4 clip_wpos;
} vertices[2];

out vec3 color;

void main()
{
	// Only extract vertices[0], since every vertex occupies both locations
	// of the array once when drawn as a line strip.
	gl_Position = vertices[0].clip_wpos;
	gl_PointSize = ctrl_color_size.w;
	color = ctrl_color_size.rgb;
	EmitVertex();
	EndPrimitive();
	gl_Position = vertices[0].clip_lpos;
	gl_PointSize = knot_color_size.w;
	color = knot_color_size.rgb;
	EmitVertex();
	EndPrimitive();
}
);

static char const *const fs_src = GLSL(330,

in vec3 color;
out vec4 color_out;

void main()
{
	color_out = vec4(color, 1.0);
}
);

static struct program_location const points_attribs[] = {
	{ "model_lpos", LINE_POS_ATTRIB },
	{ "model_wpos", WEIGHT_POS_ATTRIB },
	{ NULL, 0 }
};

static struct program_location const points_frags[] = {
	{ "color_out", 0 },
	{ NULL, 0 }
};

static struct program_uniform const points_uniforms[] = {
	{ "to_world", offsetof(struct points_program, to_world) },
	{ "to_clip", offsetof(struct points_program, to_clip) },
	{ "ctrl_color_size", offsetof(struct points_program, ctrl_color_size) },
	{ "knot_color_size", offsetof(struct points_program, knot_color_size) },
	{ NULL, 0 }
};

static struct shader const points_shaders[] = {
	{ (const GLchar **)&vs_src, GL_VERTEX_SHADER },
	{ (const GLchar **)&gs_src, GL_GEOMETRY_SHADER },
	{ (const GLchar **)&fs_src, GL_FRAGMENT_SHADER },
	{ NULL, 0 }
};

static struct program_def const points_def = {
	points_shaders,
	points_attribs,
	points_frags
};

int xylo_init_points_program(struct points_program *prog, struct gl_api *gl)
{
	prog->name = xylo_make_program(gl, &points_def);
	if (!prog->name) { return -1; }
	xylo_program_uniforms(gl, prog->name, prog, points_uniforms);
	return 0;
}

int xylo_term_points_program(struct points_program *prog, struct gl_api *gl)
{
	gl_get_core(gl)->DeleteProgram(prog->name);
	(void)memset(prog, 0, sizeof *prog);
	return 0;
}

void xylo_points_set_to_clip(
	struct points_program *prog,
	struct gl_api *gl,
	float const *matrix)
{
	gl_get_core(gl)->UniformMatrix4fv(prog->to_clip, 1, GL_FALSE, matrix);
}

void xylo_points_set_to_world(
	struct points_program *prog,
	struct gl_api *gl,
	float const *matrix)
{
	gl_get_core(gl)->UniformMatrix4fv(prog->to_world, 1, GL_FALSE, matrix);
}

void xylo_points_set_ctrl(
	struct points_program *prog,
	struct gl_api *gl,
	float red,
	float green,
	float blue,
	float size)
{
	gl_get_core(gl)->Uniform4f(prog->ctrl_color_size, red, green, blue, size);
}

void xylo_points_set_knot(
	struct points_program *prog,
	struct gl_api *gl,
	float red,
	float green,
	float blue,
	float size)
{
	gl_get_core(gl)->Uniform4f(prog->knot_color_size, red, green, blue, size);
}

