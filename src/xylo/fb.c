
#include <GL/gl.h>
#include <GL/glext.h>

#include <glapi/api.h>
#include <glapi/core.h>

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

int xylo_init_fb(
	struct xylo_fb *dest,
	struct gl_api *api,
	unsigned width,
	unsigned height)
{
	GLuint fbo, samptex;
	struct gl_core const *restrict gl;

	gl = gl_get_core(api);

	/* framebuffer */
	gl->GenFramebuffers(1, &fbo);
	gl->GenTextures(1, &samptex);

	gl->BindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* color buffer texture */
	gl->ActiveTexture(GL_TEXTURE0);
	gl->BindTexture(GL_TEXTURE_2D, samptex);
	gl->TexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, samptex, 0);

	dest->fbo = fbo;
	dest->samptex = samptex;
	return 0;
}

int xylo_term_fb(struct xylo_fb *fb, struct gl_api *api)
{
	struct gl_core const *restrict gl;

	gl = gl_get_core(api);

	gl->DeleteFramebuffers(1, &fb->fbo);
	gl->DeleteTextures(1, &fb->samptex);

	fb->fbo = 0;
	fb->samptex = 0;
	fb->width = fb->height = 0;

	return 0;
}

