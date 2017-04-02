
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

// Each incoming vertex embeds two spline verticies: the on-curve point (lpos)
// and the off-curve point (wpos).
in vec2 model_lpos;
in vec2 model_wpos;
in float weight;

uniform mat4 to_world;
uniform mat4 to_clip;
uniform vec2 center = vec2(0.0, 0.0);

out vertex_layout
{
	vec4 clip_center;
	vec4 clip_lpos;
	vec4 clip_wpos;
	float weight;
} vertex;

void main()
{
	// Transform both incoming points and assemble them into polygons in
	// the geometry shader.
	mat4 m = to_clip * to_world;

	vertex.clip_center = m * vec4(center, 0.0, 1.0);
	vertex.clip_lpos = m * vec4(model_lpos, 0.0, 1.0);
	vertex.clip_wpos = m * vec4(model_wpos, 0.0, 1.0);
	vertex.weight = weight;
}
);

static char const *const gs_src = GLSL(330,

layout(lines) in;
layout(triangle_strip, max_vertices = 32) out;

uniform float frame_time = 0.0;
uniform ivec4 viewport;

in vertex_layout
{
	vec4 clip_center;
	vec4 clip_lpos;
	vec4 clip_wpos;
	float weight;
} vertices[2];

out fragment_layout
{
	vec3 coord;
	flat int bitmask;
};

// int samples = 2;

void main()
{
	// clip space vertices
	vec4 p0 = vertices[0].clip_wpos;
	vec4 p1 = vertices[0].clip_lpos;
	vec4 p2 = vertices[1].clip_lpos;
	vec4 p3 = vertices[0].clip_center;

	float pw = 2.0 / viewport.z;
	float ph = 2.0 / viewport.w;

	float sx = pw / 3.0;
	float sy = -ph / 3.0;

	// Perspective corrected quadratic curve vertices
	float w = vertices[0].weight;
	vec3 q0 = vec3(0.5/w, 0.0, 1.0/w);

	// Shared vertices
	vec3 q1 = vec3(0.0, 0.0, 1.0);
	vec3 q2 = vec3(1.0, 1.0, 1.0);

	// The second triangle is a fully internal/external part of the shape
	// and should count towards the winding number. Set its vertex location
	// to (0, 1), but anything above the x^2 curve is filled.
	vec3 q3 = vec3(0.0, 1.0, 1.0);

	float s = sin(frame_time);
	float c = cos(frame_time);
	vec4 off = vec4(s * -0.3, c * 0.3, 0.0, 0.0);

	// Create a strip of two triangles starting with the weighed off-curve
	// point. It is the only point that has a weight other than one, and
	// the second one should be flat z=1 throughout.
	gl_Position = p0 + off;
	coord = q0;
	EmitVertex();
	gl_Position = p1 + off;
	coord = q1;
	EmitVertex();
	gl_Position = p2 + off;
	coord = q2;
	EmitVertex();
	gl_Position = p3 + off;
	coord = q3;
	EmitVertex();
	EndPrimitive();

	vec4 soff = vec4(sx, sy, 0, 0);
	gl_Position = p0 + off + soff;
	coord = q0;
	EmitVertex();
	gl_Position = p1 + off + soff;
	coord = q1;
	EmitVertex();
	gl_Position = p2 + off + soff;
	coord = q2;
	EmitVertex();
	gl_Position = p3 + off + soff;
	coord = q3;
	EmitVertex();
	EndPrimitive();
}
);

static char const *fs_src = GLSL(330,

in fragment_layout
{
	vec3 coord;
	flat int bitmask;
};

out vec4 fill_mask;

void main()
{
	float k = coord.x;
	float l = coord.y;
	float m = coord.z;
	float c;

	if (k*k - l*m >= 0) { c = 0.0; } else { c = 1.0; }
	fill_mask = vec4(c, c, c, 1.0);
}
);

static struct program_location const spline_attribs[] = {
	{ "model_lpos", LINE_POS_ATTRIB },
	{ "model_wpos", WEIGHT_POS_ATTRIB },
	{ "weight", WEIGHT_VAL_ATTRIB },
	{ NULL, 0 }
};

static struct program_location const spline_frags[] = {
	{ "fill_mask", 0 },
	{ NULL, 0 }
};

static struct program_uniform const spline_uniforms[] = {
	{ "to_world", offsetof(struct spline_program, to_world) },
	{ "to_clip", offsetof(struct spline_program, to_clip) },
	{ "center", offsetof(struct spline_program, center) },
	{ "frame_time", offsetof(struct spline_program, frame_time) },
	{ "viewport", offsetof(struct spline_program, viewport) },
	{ NULL, 0 }
};

static struct shader const spline_shaders[] = {
	{ (const GLchar **)&vs_src, GL_VERTEX_SHADER },
	{ (const GLchar **)&gs_src, GL_GEOMETRY_SHADER },
	{ (const GLchar **)&fs_src, GL_FRAGMENT_SHADER },
	{ NULL, 0 }
};

static struct program_def const spline_def = {
	spline_shaders,
	spline_attribs,
	spline_frags
};

int xylo_init_spline_program(struct spline_program *prog, struct gl_api *gl)
{
	fprintf(stderr, "%s\n", gs_src + 445);
	prog->name = xylo_make_program(gl, &spline_def);
	if (!prog->name) { return -1; }
	xylo_program_uniforms(gl, prog->name, prog, spline_uniforms);
	return 0;
}

int xylo_term_spline_program(struct spline_program *prog, struct gl_api *gl)
{
	gl_get_core(gl)->DeleteProgram(prog->name);
	(void)memset(prog, 0, sizeof *prog);
	return 0;
}

void xylo_spline_set_to_clip(
	struct spline_program *prog,
	struct gl_api *gl,
	float const *matrix)
{
	gl_get_core(gl)->UniformMatrix4fv(prog->to_clip, 1, GL_FALSE, matrix);
}

void xylo_spline_set_to_world(
	struct spline_program *prog,
	struct gl_api *gl,
	float const *matrix)
{
	gl_get_core(gl)->UniformMatrix4fv(prog->to_world, 1, GL_FALSE, matrix);
}

void xylo_spline_set_center(
	struct spline_program *prog,
	struct gl_api *gl,
	float x,
	float y)
{
	gl_get_core(gl)->Uniform2f(prog->center, x, y);
}

void xylo_spline_set_frame_time(
	struct spline_program *prog,
	struct gl_api *gl,
	float t)
{
	gl_get_core(gl)->Uniform1f(prog->frame_time, t);
}

void xylo_spline_set_viewport(
	struct spline_program *prog,
	struct gl_api *gl,
	int x,
	int y,
	int w,
	int h)
{
	gl_get_core(gl)->Uniform4i(prog->viewport, x, y, w, h);
}
