
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

#include "../types.h"
#include "../include/xtypes.h"
#include "../decl.h"
#include "../include/x.h"

#include "private.h"

static struct gl_xconfig const default_config = {
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
	int i, n, best_i, sb, samples, best_samples;
	GLXFBConfig *fbconfigs, fbconfig;

	fbconfigs = glXChooseFBConfig(dpy, screen, attr, &n);
	if (!fbconfigs) { return -1; }

	for (best_i = -1, i = 0; i < n; i++) {
		fbconfig = fbconfigs[i];
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

static int context_flags(struct gl_xconfig const *config)
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

static int drawable_flags(struct gl_xconfig const *config)
{
	int flags = 0;
	if (config->window) { flags |= GLX_WINDOW_BIT; }
	if (config->pixmap) { flags |= GLX_PIXMAP_BIT; }
	if (config->pbuffer) { flags |= GLX_PBUFFER_BIT; }
	return flags;
}

struct gl_xstate *gl_make_xcontext_buf(
	struct gl_xstate *xstate,
	Display *display,
	struct gl_xconfig const *config)
{
	typedef GLXContext create_context_fn(
		Display *,
		GLXFBConfig,
		GLXContext,
		Bool,
		const int *);

	struct gl_xconfig const *cfg = config ? config : &default_config;

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
	assert(xstate != NULL);

	/* Only support OpenGL core 3.3+ */
	if (cfg->vmajor < 3 || (cfg->vmajor == 3 && cfg->vminor < 3)) {
		return NULL;
	}
	if (check_glxversion(display)) { return NULL; }

	screen = DefaultScreen(display);
	glxexts = glXQueryExtensionsString(display, screen);
	if (!gl_is_extension_supported(glxexts, "GLX_ARB_create_context")) {
		return NULL;
	}
	if (select_fbconfig(display, screen, fbconfig_attribs, &fbconfig)) {
		return NULL;
	}

	glXCreateContextAttribsARB = (create_context_fn *)glXGetProcAddressARB(
		(const GLubyte *)"glXCreateContextAttribsARB");
 
	/* TODO: error handling with XSetErrorHandler in case context creation
	   fails. */

	context = glXCreateContextAttribsARB(
		display,
		fbconfig,
		0,
		True,
		context_attribs);
 
	// Sync to ensure any errors generated are processed.
	(void)XSync(display, False);

	xstate->display = display;
	xstate->fbconfig = fbconfig;
	xstate->context = context;
	hmap_init(
		&xstate->drawables,
		sizeof(struct gl_xdrawable),
		alignof(struct gl_xdrawable));


	gl_init_state(&xstate->state);

	return xstate;
}

struct gl_xstate *gl_make_xcontext(
	Display *display,
	struct gl_xconfig const *config)
{
	struct gl_xstate *state = malloc(sizeof *state), *p;
	if (!state) { return NULL; }
	p = gl_make_xcontext_buf(state, display, config);
	if (!p) {
		free(state);
	}
	return p;
}

void gl_destroy_drawable(
	struct gl_xstate *xstate,
	struct gl_xdrawable *drawable)
{
	drawable->destroy(xstate, drawable);
}

void gl_free_xcontext(struct gl_xstate *xstate)
{
	struct hmap_bucket *b;
	struct hmap *drawables;
	struct gl_xdrawable *d;

	drawables = &xstate->drawables;
	for (b = hmap_first(drawables); b; b = hmap_next(drawables, b)) {
		d = hmap_value(drawables, b);
		d->destroy(xstate, d);
	}
	hmap_term(drawables);
	glXDestroyContext(xstate->display, xstate->context);
	assert(gl_free_state(&xstate->state) == 0);
}

static void unmake_current(struct gl_xstate *xstate, GLXDrawable drawable)
{
	GLXDrawable write = glXGetCurrentDrawable();
	if (drawable == write) {
		GLXDrawable read = glXGetCurrentReadDrawable();
		if (read == drawable) { read = None; }
		glXMakeContextCurrent(
			xstate->display,
			None,
			read,
			xstate->context);
	}
}

static void destroy_glxwindow(
	struct gl_xstate *xstate,
	struct gl_xdrawable *drawable)
{
	unmake_current(xstate, drawable->glxid);
	glXDestroyWindow(xstate->display, drawable->glxid);
}

struct gl_xdrawable *gl_add_xwindow(struct gl_xstate *xstate, Window window)
{
	struct hmap *drawables;
	struct gl_xdrawable *drawable;
	GLXDrawable id;

	drawables = &xstate->drawables;
	drawable = hmap_new(drawables, &window, sizeof window);
	if (!drawable) {
		/* X window already added (or allocation failure). Should we
		   return the current binding here? */
		return NULL;
	}
	id = glXCreateWindow(xstate->display, xstate->fbconfig, window, NULL);
	drawable->xid = window;
	drawable->glxid = id;
	drawable->destroy = destroy_glxwindow;
	return drawable;
}

static void destroy_pixmap(
	struct gl_xstate *xstate,
	struct gl_xdrawable *drawable)
{
	unmake_current(xstate, drawable->glxid);
	glXDestroyPixmap(xstate->display, drawable->glxid);
}

struct gl_xdrawable *gl_add_xpixmap(struct gl_xstate *xstate, Pixmap pixmap)
{
	struct hmap *drawables;
	struct gl_xdrawable *drawable;
	GLXDrawable id;

	drawables = &xstate->drawables;
	drawable = hmap_new(drawables, &pixmap, sizeof pixmap);
	if (!drawable) {
		/* Pixmap already added (or allocation failure). Should we
		   return the current binding here? */
		return NULL;
	}
	id = glXCreatePixmap(xstate->display, xstate->fbconfig, pixmap, NULL);
	drawable->xid = pixmap;
	drawable->glxid = id;
	drawable->destroy = destroy_pixmap;
	return drawable;
}

int gl_make_current(struct gl_xstate *xstate, struct gl_xdrawable *drawable)
{
	return glXMakeContextCurrent(
		xstate->display,
		drawable->glxid,
		drawable->glxid,
		xstate->context) ? 0 : -1;
}

XVisualInfo *gl_visual_info(struct gl_xstate *xstate)
{
	return glXGetVisualFromFBConfig(xstate->display, xstate->fbconfig);
}

void gl_swap_buffers(struct gl_xstate *xstate, struct gl_xdrawable *window)
{
	(void)glXSwapBuffers(xstate->display, window->glxid);
}

