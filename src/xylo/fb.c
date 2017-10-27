#include <assert.h>
#include <stdlib.h>

#include <glapi/core.h>

#include "types.h"
#include "fb.h"

void xylo_init_fb(
	struct gl_core33 const *restrict gl,
	struct xylo_fb *fb,
	GLsizei width,
	GLsizei height)
{
	GLenum ifmt, fmt, type;
	GLint level;

	assert(gl != NULL);
	assert(fb != NULL);

	fb->width = width;
	fb->height = height;

	if (width <= 0 || height <= 0) { return; }

	/* Texture */
	ifmt = GL_RGBA8;
	fmt = GL_RGBA;
	type = GL_UNSIGNED_BYTE;
	level = 0;

	gl->GenTextures(1, &fb->tex);
	gl->BindTexture(GL_TEXTURE_2D, fb->tex);
	gl->TexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, fmt, type, 0);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Renderbuffer */
	gl->GenRenderbuffers(1, &fb->ds);
	gl->BindRenderbuffer(GL_RENDERBUFFER, fb->ds);
	gl->RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);

	gl->GenFramebuffers(1, &fb->fbo);
	gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, fb->fbo);

	gl->FramebufferTexture2D(
		GL_DRAW_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		fb->tex,
		level);

	gl->FramebufferRenderbuffer(
		GL_DRAW_FRAMEBUFFER,
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER,
		fb->ds);
}

void xylo_fb_resize(
	struct gl_core33 const *restrict gl,
	struct xylo_fb *fb,
	GLsizei width,
	GLsizei height)
{
	xylo_term_fb(gl, fb);
	xylo_init_fb(gl, fb, width, height);
}

void xylo_term_fb(struct gl_core33 const *restrict gl, struct xylo_fb *fb)
{
	if (fb->width > 0 && fb->height > 0) {
		gl->DeleteFramebuffers(1, &fb->fbo);
		gl->DeleteRenderbuffers(1, &fb->ds);
		gl->DeleteTextures(1, &fb->tex);
	}
}
