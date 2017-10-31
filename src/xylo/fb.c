#include <assert.h>
#include <stdlib.h>

#include <glapi/core.h>

#include "types.h"
#include "fb.h"

void xylo_init_fb(
	struct gl_core33 const *restrict gl,
	struct xylo_fb *fb,
	_Bool object_buffer)
{
	GLuint tex[2];

	assert(gl != NULL);
	assert(fb != NULL);

	gl->GenFramebuffers(1, &fb->fbo);
	gl->GenTextures(1 + object_buffer, tex);
	gl->GenRenderbuffers(1, &fb->ds);
	fb->color = tex[0];
	fb->object_id = object_buffer ? tex[1] : 0;
	fb->height = fb->width = -1;
}

static GLenum const drawbuffers[] = {
	[0] = GL_COLOR_ATTACHMENT0,
	[1] = GL_COLOR_ATTACHMENT1,
};

void xylo_fb_resize(
	struct gl_core33 const *restrict gl,
	struct xylo_fb *fb,
	GLsizei width,
	GLsizei height)
{
	GLenum ifmt, fmt, type, target;
	GLint level;

	assert(gl != NULL);
	assert(fb != NULL);

	if (width == fb->width && height == fb->height) { return; }

	fb->width = width;
	fb->height = height;
	if (width <= 0 || height <= 0) { return; }

	target = GL_TEXTURE_2D;
	level = 0;

	ifmt = GL_RGBA8;
	fmt = GL_RGBA;
	type = GL_UNSIGNED_BYTE;

	gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, fb->fbo);
	gl->DrawBuffers(fb->object_id ? 2 : 1, drawbuffers);

	gl->BindTexture(target, fb->color);
	gl->TexImage2D(target, level, ifmt, width, height, 0, fmt, type, 0);
	gl->TexParameteri(target, GL_TEXTURE_MAX_LEVEL, level);
	gl->FramebufferTexture2D(
		GL_DRAW_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		target,
		fb->color,
		level);

	if (fb->object_id) {
		ifmt = GL_R16UI;
		fmt = GL_RED_INTEGER;
		type = GL_UNSIGNED_SHORT;
		gl->BindTexture(target, fb->object_id);
		gl->TexImage2D(target, level, ifmt, width, height, 0, fmt, type, 0);
		gl->TexParameteri(target, GL_TEXTURE_MAX_LEVEL, level);
		gl->FramebufferTexture2D(
			GL_DRAW_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT1,
			target,
			fb->object_id,
			level);
	}

	ifmt = GL_DEPTH_STENCIL;
	gl->BindRenderbuffer(GL_RENDERBUFFER, fb->ds);
	gl->RenderbufferStorage(GL_RENDERBUFFER, ifmt, width, height);
	gl->FramebufferRenderbuffer(
		GL_DRAW_FRAMEBUFFER,
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER,
		fb->ds);
}

unsigned xylo_fb_object_id(
	struct gl_core33 const *restrict gl,
	struct xylo_fb *fb,
	GLsizei x,
	GLsizei y)
{
	GLushort value;

	if (fb->object_id == 0) { return 0; }
	if (x < 0 || x >= fb->width) { return 0; }
	if (y < 0 || y >= fb->height) { return 0; }

	gl->BindFramebuffer(GL_READ_FRAMEBUFFER, fb->fbo);
	gl->ReadBuffer(GL_COLOR_ATTACHMENT1);
	gl->ReadPixels(
		x, y,
		1, 1,
		GL_RED_INTEGER,
		GL_UNSIGNED_SHORT,
		&value);

	return value;
}

void xylo_term_fb(struct gl_core33 const *restrict gl, struct xylo_fb *fb)
{
	assert(gl != NULL);
	assert(fb != NULL);
	if (fb->width > 0 && fb->height > 0) {
		GLuint tex[2] = { fb->color, fb->object_id };
		gl->DeleteFramebuffers(1, &fb->fbo);
		gl->DeleteRenderbuffers(1, &fb->ds);
		gl->DeleteTextures(fb->object_id ? 2 : 1, tex);
	}
}
