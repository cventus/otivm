#include <assert.h>
#include <stdlib.h>

#include <glapi/api.h>
#include <glapi/core.h>
#include <glam/program.h>

#include "include/types.h"
#include "include/xylo.h"
#include "private.h"
#include "types.h"
#include "xylo.h"
#include "quincunx.h"

static struct gl_shader_source const quincunx_vert = {
	GL_VERTEX_SHADER,
	GLSL(330,
	out vec2 center_coord;
	out vec2 corner_coord;

	uniform vec2 pixel_size;

	void main()
	{
		float pw = pixel_size.x;
		float ph = pixel_size.y;
		float phw = pixel_size.x * 0.5;
		float phh = pixel_size.y * 0.5;

		switch (gl_VertexID) {
		/* top left */
		case 0:	center_coord = vec2(0.0, 1.0 - ph);
			corner_coord = vec2(0.5 + phw, 1.0 - phh);
			gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
			break;

		/* bottom left */
		case 1:	center_coord = vec2(0.0, 0.0);
			corner_coord = vec2(0.5 + phw, 0.0 + phh);
			gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
			break;

		/* top right */
		case 2:	center_coord = vec2(0.5 - pw, 1.0 - ph);
			corner_coord = vec2(1.0 - phw, 1.0 - phh);
			gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
			break;

		/* bottom right */
		default:
		case 3:	center_coord = vec2(0.5 - pw, 0.0);
			corner_coord = vec2(1.0 - phw, 0.0 + phh);
			gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
			break;
		}
	})
};

static struct gl_shader_source const quincunx_frag = {
	GL_FRAGMENT_SHADER,
	GLSL(330,

	uniform sampler2D tex;

	in vec2 center_coord;
	in vec2 corner_coord;

	out vec4 fill_color;

	/* how much weight to give to sample in the middle */
	const float center_samples = 4.0;

	const float total_samples = 4.0 + center_samples;
	const float corner_samples = 4.0;
	const float factor = corner_samples / total_samples;

	void main()
	{
		/* single center sample */
		vec4 center_sample = texture2D(tex, center_coord);

		/* arithmetic mean of four corner samples */
		vec4 corner_mean = texture2D(tex, corner_coord);

		/* five samples in total, but give more weight to center,
		   effectively treating it as four samples on the same
		   place */
		fill_color = mix(center_sample, corner_mean, factor);
	})
};

static struct gl_location const fragment_locs[] = {
	{ FILL_COLOR_LOC, "fill_color" },
	{ 0, 0 }
};

static struct gl_location const quincunx_attributes[] = {
	{ 0, 0 }
};

#define UNIFORM(name) { offsetof(struct xylo_quincunx, name), #name }
static struct gl_uniform_layout const uniforms[] = {
	UNIFORM(pixel_size),
	UNIFORM(tex),
	{ 0, 0 }
};
#undef UNIFORM

int xylo_init_quincunx(struct xylo_quincunx *quincunx, struct gl_api *api)
{
	struct gl_core33 const *restrict gl;
	struct gl_program_layout const program = {
		(struct gl_shader_source[]) {
			quincunx_vert,
			quincunx_frag,
			{ GL_NONE, 0 }
		},
		quincunx_attributes,
		fragment_locs
	};
	quincunx->program = gl_make_program(api, &program);
	if (!quincunx->program) { return -1; }
	gl_get_uniforms(api, quincunx, quincunx->program, uniforms);

	gl = gl_get_core33(api);
	gl->GenVertexArrays(1, &quincunx->vao);

	return 0;
}

void xylo_term_quincunx(struct xylo_quincunx *quincunx, struct gl_api *api)
{
	gl_unuse_program(api, quincunx->program);
	gl_get_core30(api)->DeleteVertexArrays(1, &quincunx->vao);
	gl_get_core30(api)->DeleteProgram(quincunx->program);
}

void xylo_quincunx_draw(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl)
{
	GLint vao;
	GLboolean stencil_test, depth_test;

	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == quincunx->program);

	stencil_test = gl->IsEnabled(GL_STENCIL_TEST);
	depth_test = gl->IsEnabled(GL_DEPTH_TEST);
	if (stencil_test) { gl->Disable(GL_STENCIL_TEST); }
	if (depth_test) { gl->Disable(GL_DEPTH_TEST); }
	gl->GetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
	if (!vao) {
		/* a VAO is needed to be bound before calling glDrawArrays even
		   if is empty/no enabled attributes */
		gl->BindVertexArray(quincunx->vao);
	}
	gl->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	if (!vao) { gl->BindVertexArray(0); }
	if (stencil_test) { gl->Enable(GL_STENCIL_TEST); }
	if (depth_test) { gl->Enable(GL_STENCIL_TEST); }
}

void xylo_quincunx_set_pixel_size(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	float x_size, float y_size)
{
	float values[] = { x_size, y_size };
	xylo_quincunx_set_pixel_size2fv(quincunx, gl, values);
}

void xylo_quincunx_set_pixel_size2fv(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	float const *pixel_size)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == quincunx->program);
	gl->Uniform2fv(quincunx->pixel_size, 1, pixel_size);
}

void xylo_quincunx_set_tex_unit(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	GLuint unit)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == quincunx->program);
	gl->Uniform1i(quincunx->tex, unit);
}
