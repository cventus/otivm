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
	in vec2 model_lpos;
	in vec2 model_wpos;
	in float weight;

	uniform mat4 mvp;

	out vertex_data
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
		vertex.clip_lpos = mvp * vec4(model_lpos, 0.0, 1.0);
		vertex.clip_wpos = mvp * vec4(model_wpos, 0.0, 1.0);
		vertex.weight = weight;
	})
};

static struct gl_shader_source const geometry_shader = {
	GL_GEOMETRY_SHADER,
	GLSL(330,
	layout(lines) in;
	layout(triangle_strip, max_vertices = 4) out;

	uniform vec4 center = vec4(0.0, 0.0, 0.0, 1.0);

	in vertex_data
	{
		vec4 clip_lpos;
		vec4 clip_wpos;
		float weight;
	} vertices[2];

	out fragment_data
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
	})
};

static struct gl_shader_source const fragment_shader = {
	GL_FRAGMENT_SHADER,
	GLSL(330,

	uniform vec4 color;

	in fragment_data
	{
		vec3 coord;
	};

	out vec4 fill_color;

	void main()
	{
		float k = coord.x;
		float l = coord.y;
		float m = coord.z;
		float c;

		if (k*k - l*m >= 0.0f) { c = 0.0; } else { c = 1.0; }
		fill_color = vec4(c, c, c, 1.0);
	}),
};

enum
{
	FILL_COLOR_LOC = 0
};

static struct gl_location const shape_attributes[] = {
	{ LINE_POS_ATTRIB, "model_lpos" },
	{ WEIGHT_POS_ATTRIB, "model_wpos" },
	{ WEIGHT_VAL_ATTRIB, "weight" },
	{ 0, 0 }
};

#define UNIFORM(name) { offsetof(struct xylo_uniforms, name), #name }
static struct gl_uniform_layout const uniforms[] = {
	UNIFORM(mvp),
	UNIFORM(center),
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
			geometry_shader,
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
	gl_get_core33(xylo->api)->BindVertexArray(set->vao);
}

void xylo_set_color4fv(struct xylo *xylo, float const *color)
{
	gl_get_core33(xylo->api)->Uniform4fv(xylo->uniforms.color, 1, color);
}

void xylo_set_mvp(struct xylo *xylo, float const *mvp)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	float center[4], origin[4] = { 0.f, 0.f, 0.f, 1.f };
	gl->Uniform4fv(xylo->uniforms.center, 1, m44mulvf(center, mvp, origin));
	gl->UniformMatrix4fv(xylo->uniforms.mvp, 1, GL_FALSE, mvp);
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
