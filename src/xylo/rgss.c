#include <assert.h>
#include <stdlib.h>

#include "glapi/api.h"
#include "glapi/core.h"
#include "glam/program.h"
#include "xylo/types.h"
#include "xylo/xylo.h"

#include "private.h"
#include "types.h"
#include "xylo.h"
#include "aa.h"

static struct gl_shader_source const rgss_vert = {
	GL_VERTEX_SHADER,
	GLSL(330,
	out vec2 sample1;
	out vec2 sample2;
	out vec2 sample3;
	out vec2 sample4;

	void main()
	{
		switch (gl_VertexID) {
		/* top left */
		case 0:	sample1 = vec2(0.0, 1.0);
			sample2 = vec2(0.5, 1.0);
			sample3 = vec2(0.0, 0.5);
			sample4 = vec2(0.5, 0.5);
			gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
			break;

		/* bottom left */
		case 1:	sample1 = vec2(0.0, 0.5);
			sample2 = vec2(0.5, 0.5);
			sample3 = vec2(0.0, 0.0);
			sample4 = vec2(0.5, 0.0);
			gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
			break;

		/* top right */
		case 2:	sample1 = vec2(0.5, 1.0);
			sample2 = vec2(1.0, 1.0);
			sample3 = vec2(0.5, 0.5);
			sample4 = vec2(1.0, 0.5);
			gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
			break;

		/* bottom right */
		default:
		case 3:	sample1 = vec2(0.5, 0.5);
			sample2 = vec2(1.0, 0.5);
			sample3 = vec2(0.5, 0.0);
			sample4 = vec2(1.0, 0.0);
			gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
			break;
		}
	})
};

static struct gl_shader_source const rgss_frag = {
	GL_FRAGMENT_SHADER,
	GLSL(330,

	uniform sampler2D tex;

	in vec2 sample1;
	in vec2 sample2;
	in vec2 sample3;
	in vec2 sample4;

	out vec4 fill_color;

	void main()
	{
		/* arithmetic mean of all samples */
		fill_color = 0.25 * texture2D(tex, sample1);
		fill_color += 0.25 * texture2D(tex, sample2);
		fill_color += 0.25 * texture2D(tex, sample3);
		fill_color += 0.25 * texture2D(tex, sample4);
	}
	)
};

static struct gl_location const fragment_locs[] = {
	{ FILL_COLOR_LOC, "fill_color" },
	{ 0, 0 }
};

static struct gl_location const rgss_attributes[] = {
	{ 0, 0 }
};

#define UNIFORM(name) { offsetof(struct xylo_rgss, name), #name }
static struct gl_uniform_layout const uniforms[] = {
	UNIFORM(tex),
	{ 0, 0 }
};
#undef UNIFORM

int xylo_init_rgss(struct xylo_rgss *rgss, struct gl_api *api)
{
	struct gl_core33 const *restrict gl;
	struct gl_program_layout const program = {
		(struct gl_shader_source[]) {
			rgss_vert,
			rgss_frag,
			{ GL_NONE, 0 }
		},
		rgss_attributes,
		fragment_locs
	};
	rgss->program = gl_make_program(api, &program);
	if (!rgss->program) { return -1; }
	gl_get_uniforms(api, rgss, rgss->program, uniforms);

	gl = gl_get_core33(api);
	gl->GenVertexArrays(1, &rgss->vao);

	return 0;
}

void xylo_term_rgss(
	struct xylo_rgss *rgss,
	struct gl_api *api)
{
	gl_unuse_program(api, rgss->program);
	gl_get_core30(api)->DeleteVertexArrays(1, &rgss->vao);
	gl_get_core30(api)->DeleteProgram(rgss->program);
}

void xylo_rgss_draw(struct xylo_rgss *rgss, struct gl_core33 const *restrict gl)
{
	GLint vao;
	GLboolean stencil_test, depth_test;

	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == rgss->program);

	stencil_test = gl->IsEnabled(GL_STENCIL_TEST);
	depth_test = gl->IsEnabled(GL_DEPTH_TEST);
	if (stencil_test) { gl->Disable(GL_STENCIL_TEST); }
	if (depth_test) { gl->Disable(GL_DEPTH_TEST); }
	gl->GetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
	if (!vao) {
		/* a VAO is needed to be bound before calling glDrawArrays even
		   if is empty/no enabled attributes */
		gl->BindVertexArray(rgss->vao);
	}
	gl->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	if (!vao) { gl->BindVertexArray(0); }
	if (stencil_test) { gl->Enable(GL_STENCIL_TEST); }
	if (depth_test) { gl->Enable(GL_STENCIL_TEST); }
}

void xylo_rgss_set_tex_unit(
	struct xylo_rgss *rgss,
	struct gl_core33 const *restrict gl,
	GLuint unit)
{
	assert(xylo_get_uint(gl, GL_CURRENT_PROGRAM) == rgss->program);
	gl->Uniform1i(rgss->tex, unit);
}
