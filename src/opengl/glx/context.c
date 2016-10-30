
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdalign.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <adt/hmap.h>
#include <base/mem.h>

#include "../include/core.h"
#include "../include/dbgmsg.h"

#include "../include/xtypes.h"
#include "../include/x.h"
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

	.sample_buffers = 0,
	.samples = 0
};

static int check_glxversion(Display *display)
{
	int major, minor;
	if (!glXQueryVersion(display, &major, &minor)) { return -1; }
	return (major > 1 || (major == 1 && minor >= 3)) ? 0 : -1;
}

static int select_fbconfig(
	Display *dpy,
	int screen,
	int const *attr,
	GLXFBConfig *result)
{
	int i, n, best_i, sb, samples, best_samples, depth;
	GLXFBConfig *fbconfigs, fbconfig;

	fbconfigs = glXChooseFBConfig(dpy, screen, attr, &n);
	if (!fbconfigs) { return -1; }

	for (best_i = -1, i = 0; i < n; i++) {
		fbconfig = fbconfigs[i];

		glXGetFBConfigAttrib(dpy, fbconfig, GLX_DEPTH_SIZE, &depth);
		if (depth == 0) { continue; }

		glXGetFBConfigAttrib(dpy, fbconfig, GLX_SAMPLE_BUFFERS, &sb);
		glXGetFBConfigAttrib(dpy, fbconfig, GLX_SAMPLES, &samples);
		if (best_i < 0 || (sb && samples > best_samples)) {
			best_i = i;
			best_samples = samples;
		}
	}
	if (best_i >= 0) *result = fbconfigs[best_i];
	XFree(fbconfigs);
	return best_i >= 0 ? 0 : -1;
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

static int drawable_flags(struct glx_config const *config)
{
	int flags = 0;
	if (config->window) { flags |= GLX_WINDOW_BIT; }
	if (config->pixmap) { flags |= GLX_PIXMAP_BIT; }
	if (config->pbuffer) { flags |= GLX_PBUFFER_BIT; }
	return flags;
}

struct glx *glx_init(
	struct glx *glx,
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

	int const fbconfig_attribs[] = {
		GLX_DOUBLEBUFFER, True,
		GLX_DRAWABLE_TYPE, drawable_flags(cfg),
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		None
	};

	create_context_fn *glXCreateContextAttribsARB;
	const char *glxexts;
	GLXFBConfig fbconfig;
	GLXContext context;
	int screen;

	assert(display != NULL);
	assert(glx != NULL);

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
	if (select_fbconfig(display, screen, fbconfig_attribs, &fbconfig)) {
		return NULL;
	}

	glXCreateContextAttribsARB = (create_context_fn *)glXGetProcAddressARB(
		(const GLubyte *)"glXCreateContextAttribsARB");

	/* TODO: error handling with XSetErrorHandler in case context creation
	   fails? */

	context = glXCreateContextAttribsARB(
		display,
		fbconfig,
		0,
		True,
		context_attribs);

	// Sync to ensure any errors generated are processed.
	(void)XSync(display, False);

	if (context) {
		glx->display = display;
		glx->fbconfig = fbconfig;
		glx->context = context;

		hmap_init(
			&glx->drawables,
			sizeof(struct glx_drawable),
			alignof(struct glx_drawable));

		gl_init_state(&glx->state);
		return glx;
	} else {
		return NULL;
	}
}

struct glx *glx_make(
	Display *display,
	struct glx_config const *config)
{
	struct glx *state = malloc(sizeof *state), *p;
	if (!state) { return NULL; }
	p = glx_init(state, display, config);
	if (!p) {
		free(state);
	}
	return p;
}

void glx_destroy_drawable(
	struct glx *glx,
	struct glx_drawable *drawable)
{
	XID const xid = drawable->xid;
	if (!hmap_remove(&glx->drawables, &xid, sizeof xid)) {
		drawable->destroy(glx, drawable);
	}
}

void glx_term(struct glx *glx)
{
	struct hmap_bucket *b;
	struct hmap *drawables;
	struct glx_drawable *d;

	drawables = &glx->drawables;
	for (b = hmap_first(drawables); b; b = hmap_next(drawables, b)) {
		d = hmap_value(drawables, b);
		d->destroy(glx, d);
	}
	hmap_term(drawables);
	glXDestroyContext(glx->display, glx->context);
	assert(gl_term_state(&glx->state) == 0);
}

void glx_free(struct glx *glx)
{
	glx_term(glx);
	free(glx);
}

static void destroy_glxwindow(struct glx *glx, struct glx_drawable *drawable)
{
	hmap_remove(&glx->drawables, &drawable->xid, sizeof drawable->xid);
	glXDestroyWindow(glx->display, drawable->glxid);
}

struct glx_drawable *glx_make_drawable_window(struct glx *glx, Window window)
{
	struct hmap *drawables;
	struct glx_drawable *drawable;
	GLXDrawable id;

	drawables = &glx->drawables;
	drawable = hmap_new(drawables, &window, sizeof window);
	if (!drawable) {
		/* X window already added (or allocation failure). Should we
		   return the existing entry here? */
		return NULL;
	}
	id = glXCreateWindow(glx->display, glx->fbconfig, window, NULL);
	drawable->xid = window;
	drawable->glxid = id;
	drawable->destroy = destroy_glxwindow;
	return drawable;
}

static void destroy_pixmap(struct glx *glx, struct glx_drawable *drawable)
{
	hmap_remove(&glx->drawables, &drawable->xid, sizeof drawable->xid);
	glXDestroyPixmap(glx->display, drawable->glxid);
}

struct glx_drawable *glx_make_drawable_pixmap(struct glx *glx, Pixmap pixmap)
{
	struct hmap *drawables;
	struct glx_drawable *drawable;
	GLXDrawable id;

	drawables = &glx->drawables;
	drawable = hmap_new(drawables, &pixmap, sizeof pixmap);
	if (!drawable) {
		/* Pixmap already added (or allocation failure). Should we
		   return the existing entry here? */
		return NULL;
	}
	id = glXCreatePixmap(glx->display, glx->fbconfig, pixmap, NULL);
	drawable->xid = pixmap;
	drawable->glxid = id;
	drawable->destroy = destroy_pixmap;
	return drawable;
}

int glx_make_current(struct glx *glx, struct glx_drawable *drawable)
{
	return glXMakeContextCurrent(
		glx->display,
		drawable->glxid,
		drawable->glxid,
		glx->context) ? 0 : -1;
}

XVisualInfo *glx_visual_info(struct glx *glx)
{
	return glXGetVisualFromFBConfig(glx->display, glx->fbconfig);
}

struct gl_state *glx_state(struct glx *glx)
{
	return &glx->state;
}

void glx_swap_buffers(struct glx *glx, struct glx_drawable *window)
{
	glXSwapBuffers(glx->display, window->glxid);
}

int gl_is_current(struct gl_state *state)
{
	struct glx *glx = container_of(state, struct glx, state);
	return glx->context == glXGetCurrentContext();
}

