#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdalign.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include "adt/hmap.h"
#include "base/mem.h"
#include "glapi/api.h"
#include "glapi/core.h"
#include <GL/glx.h>
#include "glapi/xtypes.h"
#include "glapi/x.h"

#include "../fwd.h"
#include "../types.h"
#include "private.h"

static struct glx_config const default_config = {
	.debug = true,
	.forward_compatible = false,
	.window = true,
	.pixmap = true,
	.pbuffer = false,

	.vmajor = 3,
	.vminor = 3,

	.stencil_size = 8,
	.depth_size = 24,

	.sample_buffers = 0,
	.samples = 0
};

static int drawable_flags(struct glx_config const *config)
{
	int flags = 0;
	if (config->window) { flags |= GLX_WINDOW_BIT; }
	if (config->pixmap) { flags |= GLX_PIXMAP_BIT; }
	if (config->pbuffer) { flags |= GLX_PBUFFER_BIT; }
	return flags;
}

static int select_fbconfig(
	Display *dpy,
	int screen,
	struct glx_config const *cfg,
	GLXFBConfig *result)
{
	int n;
	GLXFBConfig *fbconfigs;

	int const attribs[] = {
		GLX_DOUBLEBUFFER, True,
		GLX_DRAWABLE_TYPE, drawable_flags(cfg),
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_SAMPLES, cfg->samples,
		GLX_SAMPLE_BUFFERS, cfg->sample_buffers,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_STENCIL_SIZE, cfg->stencil_size,
		GLX_DEPTH_SIZE, cfg->depth_size,
		None
	};

	fbconfigs = glXChooseFBConfig(dpy, screen, attribs, &n);
	if (!fbconfigs || n < 1) { return -1; } else { *result = fbconfigs[0]; }
	XFree(fbconfigs);
	return 0;
}

static int context_flags(struct glx_config const *config)
{
	int flags = 0;
	if (config->debug) {
		flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
	}
	if (config->forward_compatible) {
		flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
	}
	return flags;
}

static int check_glxversion(Display *display)
{
	int major, minor;
	if (!glXQueryVersion(display, &major, &minor)) { return -1; }
	return (major > 1 || (major == 1 && minor >= 3)) ? 0 : -1;
}

struct glx_context *glx_init_context(
	struct glx_context *ctx,
	Display *display,
	struct glx_config const *config)
{
	typedef GLXContext create_context_fn(
		Display *,
		GLXFBConfig,
		GLXContext,
		Bool,
		const int *);

	struct glx_config const *cfg = config ? config : &default_config;

	int const context_attribs[] =
	{
		GLX_CONTEXT_PROFILE_MASK_ARB,
			GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		GLX_CONTEXT_MAJOR_VERSION_ARB, cfg->vmajor,
		GLX_CONTEXT_MINOR_VERSION_ARB, cfg->vminor,
		GLX_CONTEXT_FLAGS_ARB, context_flags(cfg),
		None
	};

	create_context_fn *glXCreateContextAttribsARB;
	const char *glxexts;
	GLXFBConfig fbconfig;
	GLXContext glctx;
	int screen;

	assert(display != NULL);
	assert(ctx != NULL);

	/* Only support OpenGL core 3.3+ */
	if (cfg->vmajor < 3 || (cfg->vmajor == 3 && cfg->vminor < 3)) {
		return NULL;
	}
	if (check_glxversion(display)) { return NULL; }

	screen = DefaultScreen(display);
	glxexts = glXQueryExtensionsString(display, screen);
	if (!gl_find_ext(glxexts, "GLX_ARB_create_context")) {
		return NULL;
	}
	if (select_fbconfig(display, screen, cfg, &fbconfig)) {
		return NULL;
	}

	glXCreateContextAttribsARB = (create_context_fn *)glXGetProcAddressARB(
		(const GLubyte *)"glXCreateContextAttribsARB");

	/* TODO: error handling with XSetErrorHandler in case context creation
	   fails? */

	glctx = glXCreateContextAttribsARB(
		display,
		fbconfig,
		0,
		True,
		context_attribs);

	// Sync to ensure any errors generated are processed.
	(void)XSync(display, False);

	if (glctx) {
		ctx->display = display;
		ctx->fbconfig = fbconfig;
		ctx->context = glctx;

		hmap_init(
			&ctx->drawables,
			sizeof(struct glx_drawable),
			alignof(struct glx_drawable));

		gl_init_api(&ctx->api);
		return ctx;
	} else {
		return NULL;
	}
}

struct glx_context *glx_make_context(
	Display *display,
	struct glx_config const *config)
{
	struct glx_context *ctx = malloc(sizeof *ctx), *p;
	if (!ctx) { return NULL; }
	p = glx_init_context(ctx, display, config);
	if (!p) { free(ctx); }
	return p;
}

void glx_destroy_drawable(
	struct glx_context *ctx,
	struct glx_drawable *drawable)
{
	XID const xid = drawable->xid;
	if (!hmap_remove(&ctx->drawables, &xid, sizeof xid)) {
		drawable->destroy(ctx, drawable);
	}
}

void glx_term_context(struct glx_context *ctx)
{
	struct hmap_bucket *b;
	struct hmap *drawables;
	struct glx_drawable *d;

	glXMakeCurrent(ctx->display, None, NULL);

	drawables = &ctx->drawables;
	for (b = hmap_first(drawables); b; b = hmap_next(drawables, b)) {
		d = hmap_value(drawables, b);
		d->destroy(ctx, d);
	}
	hmap_term(drawables);
	(void)gl_term_api(&ctx->api);
	glXDestroyContext(ctx->display, ctx->context);
}

void glx_free_context(struct glx_context *ctx)
{
	glx_term_context(ctx);
	free(ctx);
}

static void destroy_glxwindow(struct glx_context *ctx, struct glx_drawable *drawable)
{
	glXDestroyWindow(ctx->display, drawable->glxid);
	hmap_remove(&ctx->drawables, &drawable->xid, sizeof drawable->xid);
}

struct glx_drawable *glx_make_drawable_window(struct glx_context *ctx, Window window)
{
	struct hmap *drawables;
	struct glx_drawable *drawable;
	GLXDrawable id;

	drawables = &ctx->drawables;
	drawable = hmap_new(drawables, &window, sizeof window);
	if (!drawable) {
		/* X window already added (or allocation failure). Should we
		   return the existing entry here? */
		return NULL;
	}
	id = glXCreateWindow(ctx->display, ctx->fbconfig, window, NULL);
	drawable->xid = window;
	drawable->glxid = id;
	drawable->destroy = destroy_glxwindow;
	return drawable;
}

static void destroy_pixmap(struct glx_context *ctx, struct glx_drawable *drawable)
{
	glXDestroyPixmap(ctx->display, drawable->glxid);
	hmap_remove(&ctx->drawables, &drawable->xid, sizeof drawable->xid);
}

struct glx_drawable *glx_make_drawable_pixmap(struct glx_context *ctx, Pixmap pixmap)
{
	struct hmap *drawables;
	struct glx_drawable *drawable;
	GLXDrawable id;

	drawables = &ctx->drawables;
	drawable = hmap_new(drawables, &pixmap, sizeof pixmap);
	if (!drawable) {
		/* Pixmap already added (or allocation failure). Should we
		   return the existing entry here? */
		return NULL;
	}
	id = glXCreatePixmap(ctx->display, ctx->fbconfig, pixmap, NULL);
	drawable->xid = pixmap;
	drawable->glxid = id;
	drawable->destroy = destroy_pixmap;
	return drawable;
}

int glx_make_current(struct glx_context *ctx, struct glx_drawable *drawable)
{
	return glXMakeCurrent(
		ctx->display,
		drawable->glxid,
		ctx->context) ? 0 : -1;
}

XVisualInfo *glx_visual_info(struct glx_context *ctx)
{
	return glXGetVisualFromFBConfig(ctx->display, ctx->fbconfig);
}

struct gl_api *glx_api(struct glx_context *ctx)
{
	return &ctx->api;
}

void glx_swap_buffers(struct glx_context *ctx, struct glx_drawable *window)
{
	glXSwapBuffers(ctx->display, window->glxid);
}

int gl_is_current(struct gl_api *api)
{
	struct glx_context *ctx = container_of(api, struct glx_context, api);
	return ctx->context == glXGetCurrentContext();
}
