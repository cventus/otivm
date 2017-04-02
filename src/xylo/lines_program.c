
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
layout(line_strip, max_vertices = 3) out;

in vertex_layout
{
	vec4 clip_lpos;
	vec4 clip_wpos;
} vertices[2];

void main()
{
	gl_Position = vertices[0].clip_lpos;
	EmitVertex();
	gl_Position = vertices[0].clip_wpos;
	EmitVertex();
	gl_Position = vertices[1].clip_lpos;
	EmitVertex();
	EndPrimitive();
}
);

static char const *const fs_src = GLSL(330,

uniform vec3 color;
out vec4 color_out;

void main()
{
	color_out = vec4(color, 1.0);
}
);

static struct program_location const lines_attribs[] = {
	{ "model_lpos", LINE_POS_ATTRIB },
	{ "model_wpos", WEIGHT_POS_ATTRIB },
	{ "weight", WEIGHT_VAL_ATTRIB },
	{ NULL, 0 }
};

static struct program_location const lines_frags[] = {
	{ "color_out", 0 },
	{ NULL, 0 }
};

static struct program_uniform const lines_uniforms[] = {
	{ "to_world", offsetof(struct lines_program, to_world) },
	{ "to_clip", offsetof(struct lines_program, to_clip) },
	{ "color", offsetof(struct lines_program, color) },
	{ NULL, 0 }
};

static struct shader const lines_shaders[] = {
	{ (const GLchar **)&vs_src, GL_VERTEX_SHADER },
	{ (const GLchar **)&gs_src, GL_GEOMETRY_SHADER },
	{ (const GLchar **)&fs_src, GL_FRAGMENT_SHADER },
	{ NULL, 0 }
};

static struct program_def const lines_def = {
	lines_shaders,
	lines_attribs,
	lines_frags
};

int xylo_init_lines_program(struct lines_program *prog, struct gl_api *gl)
{
	prog->name = xylo_make_program(gl, &lines_def);
	if (!prog->name) { return -1; }
	xylo_program_uniforms(gl, prog->name, prog, lines_uniforms);
	return 0;
}

int xylo_term_lines_program(struct lines_program *prog, struct gl_api *gl)
{
	gl_get_core(gl)->DeleteProgram(prog->name);
	(void)memset(prog, 0, sizeof *prog);
	return 0;
}

void xylo_lines_set_to_clip(
	struct lines_program *prog,
	struct gl_api *gl,
	float const *matrix)
{
	gl_get_core(gl)->UniformMatrix4fv(prog->to_clip, 1, GL_FALSE, matrix);
}

void xylo_lines_set_to_world(
	struct lines_program *prog,
	struct gl_api *gl,
	float const *matrix)
{
	gl_get_core(gl)->UniformMatrix4fv(prog->to_world, 1, GL_FALSE, matrix);
}

void xylo_lines_set_color(
	struct lines_program *prog,
	struct gl_api *gl,
	float red,
	float green,
	float blue)
{
	gl_get_core(gl)->Uniform3f(prog->color, red, green, blue);
}

