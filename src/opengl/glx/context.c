
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "../types.h"
#include "../include/xtypes.h"
#include "../decl.h"
#include "../include/x.h"

#include "xdrawable.h"

static struct glxconfig const default_config = {
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

static int context_flags(struct glxconfig const *config)
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

static int drawable_flags(struct glxconfig const *config)
{
	int flags = 0;
	if (config->window) { flags |= GLX_WINDOW_BIT; }
	if (config->pixmap) { flags |= GLX_PIXMAP_BIT; }
	if (config->pbuffer) { flags |= GLX_PBUFFER_BIT; }
	return flags;
}

struct glxstate *gl_make_xcontext(
	struct glxstate *buf,
	Display *display,
	struct glxconfig const *config)
{
	typedef GLXContext create_context_fn(
		Display *,
		GLXFBConfig,
		GLXContext,
		Bool,
		const int *);

	struct glxconfig const *cfg = config ? config : &default_config;

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
	struct glxstate *xstate;

	assert(display != NULL);

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

	xstate = buf ? buf : malloc(sizeof *xstate);
	if (xstate == NULL) {
		glXDestroyContext(display, context);
		return NULL;
	}
 
	xstate->display = display;
	xstate->fbconfig = fbconfig;
	xstate->context = context;
	xstate->drawable = NULL;
	xstate->ndrawables = 0;

	gl_init_state(&xstate->state);

	return xstate;
}

void gl_free_xcontext(struct glxstate *xstate)
{
	if (xstate->drawable) {
		xstate->drawable->destroy(xstate, xstate->drawable);
		free(xstate->drawable);
	}
	glXDestroyContext(xstate->display, xstate->context);
	assert(gl_free_state(&xstate->state) == 0);
}

static void unmake_current(struct glxstate *xstate, GLXDrawable drawable)
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
	struct glxstate *xstate,
	struct glxdrawable *drawable)
{
	unmake_current(xstate, drawable->glxid);
	glXDestroyWindow(xstate->display, drawable->glxid);
}

struct glxdrawable *gl_add_xwindow(struct glxstate *xstate, Window window)
{
	GLXDrawable id;

	/* FIXME: Only one window/pixmap supported */
	if (xstate->ndrawables != 0) { return NULL; }
	xstate->drawable = malloc(sizeof *xstate->drawable);
	if (!xstate->drawable) { return NULL; }

	id = glXCreateWindow(
		xstate->display,
		xstate->fbconfig,
		window,
		NULL);

	xstate->ndrawables++;
	xstate->drawable->xid = window;
	xstate->drawable->glxid = id;
	xstate->drawable->destroy = destroy_glxwindow;

	return xstate->drawable;
}

static void destroy_pixmap(
	struct glxstate *xstate,
	struct glxdrawable *drawable)
{
	unmake_current(xstate, drawable->glxid);
	glXDestroyPixmap(xstate->display, drawable->glxid);
}

struct glxdrawable *gl_add_xpixmap(struct glxstate *xstate, Pixmap pixmap)
{
	GLXDrawable id;

	/* FIXME: Only one window/pixmap supported */
	if (xstate->ndrawables != 0) { return NULL; }
	xstate->drawable = malloc(sizeof *xstate->drawable);
	if (!xstate->drawable) { return NULL; }

	id = glXCreatePixmap(
		xstate->display,
		xstate->fbconfig,
		pixmap,
		NULL);

	xstate->ndrawables++;
	xstate->drawable->xid = pixmap;
	xstate->drawable->glxid = id;
	xstate->drawable->destroy = destroy_pixmap;

	return xstate->drawable;
}

int gl_make_current(struct glxstate *xstate, struct glxdrawable *drawable)
{
	return glXMakeContextCurrent(
		xstate->display,
		drawable->glxid,
		drawable->glxid,
		xstate->context) ? 0 : -1;
}

XVisualInfo *gl_visual_info(struct glxstate *xstate)
{
	return glXGetVisualFromFBConfig(xstate->display, xstate->fbconfig);
}
