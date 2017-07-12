#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>
#include <string.h>
#include <base/mem.h>
#include <glapi/api.h>
#include <glapi/core.h>

#include "include/types.h"
#include "private.h"
#include "types.h"

#define GLSL(version,src) ("#version " #version " core\n" #src)

static char const *vs_src = GLSL(330,

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
	// Each vertex contains two verticies: the on-curve point (lpos) and
	// the off-curve point (wpos). Transform both. These are turned into
	// polygons in the geometry shader.
	mat4 m = to_clip * to_world;

	vertex.clip_lpos = m * vec4(model_lpos, 0.0, 1.0);
	vertex.clip_wpos = m * vec4(model_wpos, 0.0, 1.0);
	vertex.weight = weight;
}
);

static char const *gs_src = GLSL(330,

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

	// The second triangle is a fully internal/external part of the shape
	// and should count towards the winding number. Set its vertex location
	// to (0, 1), but anything above the x^2 curve is filled.
	vec3 q3 = vec3(0.0, 1.0, 1.0);

	// Create a strip of two triangles starting with the weighed off-curve
	// point. It is the only point that has a weight other than one, and
	// the second one should be flat z=1 throughout.
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
}
);

static char const *fs_src = GLSL(330,

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
}
);

enum
{
	LINE_POS_ATTRIB = 0,
	WEIGHT_POS_ATTRIB = 1,
	WEIGHT_VAL_ATTRIB = 2
};

static struct
{
	GLuint loc;
	char const *name;
} const attrib_locs[] = {
	{ LINE_POS_ATTRIB, "model_lpos" },
	{ WEIGHT_POS_ATTRIB, "model_wpos" },
	{ WEIGHT_VAL_ATTRIB, "weight" }
}, frag_locs[] = {
	{ 0, "fill_mask" }
};

static struct
{
	ptrdiff_t offset;
	char const *name;
} const uniforms[] = {
	{ offsetof(struct xylo, to_world), "to_world" },
	{ offsetof(struct xylo, to_clip), "to_clip" },
	{ offsetof(struct xylo, center), "center" }
};

static GLuint make_shader_program(struct gl_api *api);
static int get_uniforms(struct gl_api *api, GLuint program, struct xylo *dest);

struct xylo *make_xylo(struct gl_api *api)
{
	struct xylo *xylo;

	if (!gl_get_core33(api)) {
		return NULL;
	}
	
	xylo = malloc(sizeof *xylo);
	xylo->program = make_shader_program(api);
	if (!xylo->program) {
		free(xylo);
		return NULL;
	}
	if (get_uniforms(api, xylo->program, xylo) != 0) {
		free_xylo(xylo);
		return NULL;
	}
	xylo->api = api;
	return xylo;
}

void free_xylo(struct xylo *xylo)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	if (xylo->program) {
		gl->UseProgram(0);
		gl->DeleteProgram(xylo->program);
	}
	free(xylo);
}

static GLuint make_shader_program(struct gl_api *api)
{
	struct {
		GLenum type;
		GLchar const **src;
	} const shaders[] = {
		{ GL_VERTEX_SHADER, (const GLchar **)&vs_src },
		{ GL_GEOMETRY_SHADER, (const GLchar **)&gs_src },
		{ GL_FRAGMENT_SHADER, (const GLchar **)&fs_src }
	};

	struct gl_core33 const *restrict gl = gl_get_core33(api);
	char const *name;
	GLuint names[3], shader, prog, result, loc;
	GLint status;
	size_t i, j;

	result = prog = gl->CreateProgram();
	for (i = 0; i < length_of(shaders); i++) {
		shader = gl->CreateShader(shaders[i].type);
		names[i] = shader;
		gl->ShaderSource(shader, 1, shaders[i].src, NULL);
		gl->CompileShader(shader);
		gl->GetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == 0) {
			result = 0;
			goto clean;
		}
		gl->AttachShader(prog, shader);
	}
	for (j = 0; j < length_of(attrib_locs); j++) {
		loc = attrib_locs[j].loc;
		name = attrib_locs[j].name;
		gl->BindAttribLocation(prog, loc, name);
	}
	for (j = 0; j < length_of(frag_locs); j++) {
		loc = frag_locs[j].loc;
		name = frag_locs[j].name;
		gl->BindFragDataLocation(prog, loc, name);
	}
	gl->LinkProgram(prog);
	gl->GetProgramiv(prog, GL_LINK_STATUS, &status);
	while (i-- > 0) {
		gl->DetachShader(prog, names[i]);
clean:		gl->DeleteShader(names[i]);
	}
	if (status == 0) {
		gl->DeleteProgram(prog);
	}
	return result;
}

static int get_uniforms(struct gl_api *api, GLuint program, struct xylo *dest)
{
	size_t i;
	GLuint *loc;
	char const *name;
	struct gl_core33 const *restrict gl = gl_get_core33(api);

	for (i = 0; i < length_of(uniforms); i++) {
		loc = (GLuint *)((char *)dest + uniforms[i].offset);
		name = uniforms[i].name;
		*loc = gl->GetUniformLocation(program, name);
	}
	return 0;
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

void xylo_draw_shape(struct xylo *xylo, struct xylo_glshape *shape)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);
	gl->MultiDrawArrays(
		GL_LINE_LOOP,
		shape->first,
		shape->count,
		shape->drawcount);
}

static size_t xylo_count_outlines(size_t n, struct xylo_shape const shape[n])
{
	size_t i, m;
	for (m = i = 0; i < n; i++) {
		m += shape->n;
	}
	return m;
}

static size_t xylo_shape_leg_size(struct xylo_shape const *shape)
{
	size_t i, n;
	for (n = i = 0; i < shape->n; i++) {
		n += shape->outlines[i].n;
	}
	return n * sizeof(struct xylo_leg);
}

static GLuint make_shape_set_vbo(
	struct gl_api *api,
	GLenum usage,
	size_t n,
	struct xylo_shape const shapes[n])
{
	char *p;
	size_t i, j, sz;
	GLuint vbo;
	struct xylo_shape const *shape;
	struct xylo_outline const *outline;
	struct gl_core33 const *restrict gl;

	for (sz = 0, i = 0; i < n; i++) {
		// FIXME: potential for overflow not handled
		sz += xylo_shape_leg_size(shapes + i);
	}
	gl = gl_get_core33(api);
	gl->GenBuffers(1, &vbo);
	gl->BindBuffer(GL_ARRAY_BUFFER, vbo);
	gl->BufferData(GL_ARRAY_BUFFER, sz, NULL, usage);
	p = gl->MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (p) {
		for (i = 0; i < n; i++) {
			shape = shapes + i;
			for (j = 0; j < shape->n; j++) {
				outline = shape->outlines + j;
				sz = sizeof (*outline->legs) * outline->n;
				(void)memcpy(p, shape->outlines[j].legs, sz);
				p += sz;
			}
		}
		gl->UnmapBuffer(GL_ARRAY_BUFFER);
		gl->BindBuffer(GL_ARRAY_BUFFER, 0);
		return vbo;
	} else {
		gl->DeleteBuffers(1, &vbo);
		return 0;
	}
}

static struct xylo_glshape *alloc_glshapes(
	size_t n,
	struct xylo_shape const shapes[n])
{
	size_t i, j, m, glshape_ext, first_ext, size;
	struct xylo_glshape *glshapes, *glshape;
	struct xylo_shape const *shape;
	GLint *firsts, tally;
	GLsizei *counts;

	m = xylo_count_outlines(n, shapes);

	/* Create single block to hold the shapes */
	if (memblk_array(0, &glshape_ext, n, *glshapes)) { return NULL; }
	if (memblk_array(glshape_ext, &first_ext, m, *firsts)) { return NULL; }
	if (memblk_array(first_ext, &size, m, *counts)) { return NULL; }

	if (glshapes = malloc(size), !glshapes) { return NULL; }
	firsts = memblk_offset(glshapes, glshape_ext, *firsts);
	counts = memblk_offset(glshapes, first_ext, *counts);

	glshape = glshapes;
	shape = shapes;
	tally = 0;

	for (i = 0; i < n; i++) {
		glshape->first = firsts;
		glshape->count = counts;
		glshape->drawcount = shape->n;
		for (j = 0; j < shape->n; j++) {
			firsts[j] = tally;
			tally += counts[j] = shape->outlines[j].n;
		}
		firsts += shape->n;
		counts += shape->n;
		glshape++;
		shape++;
	}
	return glshapes;
}

struct xylo_glshape_set *xylo_make_glshape_set(
	struct gl_api *api,
	size_t n,
	struct xylo_shape const shapes[n])
{
	static struct {
		GLuint location;
		GLint size;
		ptrdiff_t offset;
	} const attribs[] = {
		{ LINE_POS_ATTRIB, 2, 0 },
		{ WEIGHT_POS_ATTRIB, 2, 2*sizeof(float) },
		{ WEIGHT_VAL_ATTRIB, 1, 4*sizeof(float) }
	};
	struct gl_core33 const *restrict gl;
	size_t i;
	struct xylo_glshape_set *set;
	struct xylo_glshape *glshapes;
	GLuint vao, vbo;
	GLsizei stride;

	gl = gl_get_core33(api);

	glshapes = alloc_glshapes(n, shapes);
	if (!glshapes) { return NULL; }
	vbo = make_shape_set_vbo(api, GL_STATIC_DRAW, n, shapes);
	if (!vbo) {
		free(glshapes);
		return NULL;
	}
	set = malloc(sizeof *set);
	if (!set) {
		gl->DeleteBuffers(1, &vbo);
		free(glshapes);
		return NULL;
	}

	gl->GenVertexArrays(1, &vao);
	gl->BindVertexArray(vao);
	gl->BindBuffer(GL_ARRAY_BUFFER, vbo);

	stride = sizeof(struct xylo_leg);
	for (i = 0; i < length_of(attribs); i++) {
		gl->EnableVertexAttribArray(attribs[i].location);
		gl->VertexAttribPointer(
			attribs[i].location,
			attribs[i].size,
 			GL_FLOAT,
			GL_FALSE,
			stride,
			(GLvoid const *)attribs[i].offset);
	}
	gl->BindBuffer(GL_ARRAY_BUFFER, 0);
	gl->BindVertexArray(0);

	set->vao = vao;
	set->vbo = vbo;
	set->shapes = glshapes;
	set->n = n;

	return set;
}

struct xylo_glshape *xylo_get_glshape(struct xylo_glshape_set *set, size_t i)
{
	return set->shapes + i;
}

int xylo_free_glshape_set(struct xylo_glshape_set *set, struct gl_api *api)
{
	struct gl_core33 const *restrict gl = gl_get_core33(api);

	gl->DeleteBuffers(1, &set->vbo);
	gl->DeleteVertexArrays(1, &set->vbo);
	free(set->shapes);
	free(set);
	return 0;
}
