#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <string.h>
#include <base/mem.h>

#include <glapi/api.h>
#include <glapi/core.h>
#include <glam/program.h>

#include "include/types.h"
#include "private.h"
#include "types.h"

#define GLSL(version,src) ("#version " #version " core\n" #src)

#define VERTEX_SHADER(version,src) { GL_VERTEX_SHADER, GLSL(version, src) }
#define GEOMETRY_SHADER(version,src) { GL_GEOMETRY_SHADER, GLSL(version, src) }
#define FRAGMENT_SHADER(version,src) { GL_FRAGMENT_SHADER, GLSL(version, src) }

static struct gl_shader_source const sources[] = {
	VERTEX_SHADER(330,
	in vec2 model_lpos;
	in vec2 model_wpos;
	in float weight;

	uniform mat4 to_world;
	uniform mat4 to_clip;

	out vertex_layout
	{
		vec4 clip_lpos;
		vec4 clip_wpos;
		float weight;
	} vertex;

	void main()
	{
		// Each vertex contains two verticies: the on-curve point
		// (lpos) and the off-curve point (wpos). Transform both. These
		// are turned into polygons in the geometry shader.
		mat4 m = to_clip * to_world;

		vertex.clip_lpos = m * vec4(model_lpos, 0.0, 1.0);
		vertex.clip_wpos = m * vec4(model_wpos, 0.0, 1.0);
		vertex.weight = weight;
	}),

	GEOMETRY_SHADER(330,
	layout(lines) in;
	layout(triangle_strip, max_vertices = 4) out;

	uniform vec4 center = vec4(0.0, 0.0, 0.0, 1.0);

	in vertex_layout
	{
		vec4 clip_lpos;
		vec4 clip_wpos;
		float weight;
	} vertices[2];

	out fragment_layout
	{
		vec3 coord;
	};

	void main()
	{
		// clip space vertices
		vec4 p0 = vertices[0].clip_wpos;
		vec4 p1 = vertices[0].clip_lpos;
		vec4 p2 = vertices[1].clip_lpos;
		vec4 p3 = center;

		// Perspective corrected quadratic curve vertices
		float w = vertices[0].weight;
		vec3 q0 = vec3(0.5/w, 0.0, 1.0/w);

		// Shared vertices
		vec3 q1 = vec3(0.0, 0.0, 1.0);
		vec3 q2 = vec3(1.0, 1.0, 1.0);

		// The second triangle is a fully internal/external part of the
		// shape and should count towards the winding number. Set its
		// vertex location to (0, 1), but anything above the x^2 curve
		// is filled.
		vec3 q3 = vec3(0.0, 1.0, 1.0);

		// Create a strip of two triangles starting with the weighed
		// off-curve point. It is the only point that has a weight
		// other than one, and the second one should be flat z=1
		// throughout.
		gl_Position = p0;
		coord = q0;
		EmitVertex();
		gl_Position = p1;
		coord = q1;
		EmitVertex();
		gl_Position = p2;
		coord = q2;
		EmitVertex();
		gl_Position = p3;
		coord = q3;
		EmitVertex();
		EndPrimitive();
	}),

	FRAGMENT_SHADER(330,

	in fragment_layout
	{
		vec3 coord;
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
	}),

	{ 0, 0 }
};

enum
{
	FILL_MASK_LOC = 0,
};

static struct gl_location const attrib[] = {
	{ LINE_POS_ATTRIB, "model_lpos" },
	{ WEIGHT_POS_ATTRIB, "model_wpos" },
	{ WEIGHT_VAL_ATTRIB, "weight" },
	{ 0, 0 }
};

static struct gl_location const frag[] = {
	{ FILL_MASK_LOC, "fill_mask" },
	{ 0, 0 }
};

static struct gl_program_layout const prog = { sources, attrib, frag };

#define UNIFORM(name) { offsetof(struct xylo, name), #name }

static struct gl_uniform_layout const uniforms[] = {
	UNIFORM(to_world),
	UNIFORM(to_clip),
	UNIFORM(center),
	{ 0, 0 }
};

struct xylo *make_xylo(struct gl_api *api)
{
	struct xylo *xylo;

	if (!gl_get_core33(api)) {
		return NULL;
	}
	xylo = malloc(sizeof *xylo);
	xylo->program = gl_make_program(api, &prog);
	if (!xylo->program) {
		free(xylo);
		return NULL;
	}
	gl_get_uniforms(api, xylo, xylo->program, uniforms);
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

	gl->Enable(GL_COLOR_LOGIC_OP);
	gl->LogicOp(GL_XOR);
	gl->UseProgram(xylo->program);
}

void xylo_end(struct xylo *xylo)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl->UseProgram(0);
	gl->Disable(GL_COLOR_LOGIC_OP);
}

void xylo_set_shape_set(struct xylo *xylo, struct xylo_glshape_set *set)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl->BindVertexArray(set->vao);
}

void xylo_set_to_clip(struct xylo *xylo, float const *matrix)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl->UniformMatrix4fv(xylo->to_clip, 1, GL_FALSE, matrix);
}

void xylo_set_to_world(struct xylo *xylo, float const *matrix)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl->UniformMatrix4fv(xylo->to_world, 1, GL_FALSE, matrix);
}

void xylo_draw_glshape(struct xylo *xylo, struct xylo_glshape const *shape)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl->MultiDrawArrays(
		GL_LINE_LOOP,
		shape->first,
		shape->count,
		shape->drawcount);
}
