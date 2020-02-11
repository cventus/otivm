#include <GL/gl.h>
#include <GL/glext.h>

#include "glapi/api.h"
#include "glapi/core.h"

struct plain_box
{
	float pos[3];
	unsigned char color[4];
	float size[2], rcs[2]; /* rcs = scaled sin/cos = complex = 2x2 matrix */
};

/*
GL_FRAMEBUFFER
GL_READ_FRAMEBUFFER -- glReadPixels
GL_DRAW_FRAMEBUFFER -- every draw command

 only non-compressed image formats 
GL_COLOR_ATTACHMENTi, 0 <= i < 8 or glGet(GL_MAX_COLOR_ATTACHMENTS)

GL_DEPTH_ATTACHMEN -- only one, only depth formats
GL_STENCIL_ATTACHMENT -- only one, only stencil formats
GL_DEPTH_STENCIL_ATTACHMENT -- only one, combined image format

GL_RENDERBUFFER
*/

struct framebuffer
{
	GLuint fbo, color_tex, id_tex, depth_stencil_rb;
	unsigned width, height;
};

int init_framebuffer(
	struct framebuffer *dest,
	struct gl_api *api,
	unsigned width,
	unsigned height)
{
	GLuint fbo, tex[2], ds;
	struct gl_core const *restrict gl;

	gl = gl_get_core(api);

	/* framebuffer */
	gl->GenFramebuffers(2, &fbo);
	gl->GenTextures(2, tex);
	gl->GenRenderbuffers(1, &ds);

	gl->BindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* color buffer texture */
	gl->ActiveTexture(GL_TEXTURE0);
	gl->BindTexture(GL_TEXTURE_2D, tex[0]);
	gl->TexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		width,
		height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		NULL);
	gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex[0], 0);

	/* object index texture */
	gl->BindTexture(GL_TEXTURE_2D, tex[0]);
	gl->TexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_R32UI,
		width,
		height,
		0,
		GL_RED_INTEGER,
		GL_UNSIGNED_INT,
		NULL);
	gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, GL_TEXTURE_2D, tex[1], 0);

	/* depth/stencil renderbuffer */
	gl->BindRenderbuffer(GL_RENDERBUFFER, ds);
	gl->RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	gl->FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ds);

	dest->fbo = fbo;
	dest->color_tex = tex[0];
	dest->id_tex = tex[1];
	dest->depth_stencil_rb = ds;

	return 0;
}

int term_framebuffer(
	struct framebuffer *fb,
	struct gl_api *api)
{
	struct gl_core const *restrict gl;

	gl = gl_get_core(api);

	gl->BindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

	gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 0, GL_TEXTURE_2D, 0, 0);
	gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, GL_TEXTURE_2D, 0, 0);
	gl->FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);

	/* color buffer texture */
	gl->DeleteTextures(2, (GLuint const []){ fb->color_tex, fb->id_tex });
	gl->DeleteRenderbuffers(1, &fb->depth_stencil_rb);

	/* framebuffer */
	gl->DeleteFramebuffers(1, &fb->fbo);

	fb->fbo = 0;
	fb->color_tex = 0;
	fb->depth_stencil_rb = 0;
	fb->width = fb->height = 0;

	return 0;
}
