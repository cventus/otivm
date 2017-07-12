#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#define WIDTH 600
#define HEIGHT WIDTH

#include <adt/hmap.h>

#include "../include/api.h"
#include "../include/core.h"
#include <GL/glx.h>

#include "../include/xtypes.h"
#include "../fwd.h"
#include "../types.h"
#include "../include/x.h"

#include "private.h"

struct gl_test
{
	Display *display;
	XID xres;
	struct glx_drawable *drawable;
	struct glx_context ctx;
	int (*free_xres)(Display *, XID);
	void (*swap_buffers)(struct gl_test *);
	int interactive;
};

struct gl_api *gl_test_api(struct gl_test *test)
{
	assert(test);
	return &test->ctx.api;
}

static void swap_buffers(struct gl_test *test)
{
	assert(test);
	glXSwapBuffers(test->display, test->drawable->glxid);
}

static void dont_swap_buffers(struct gl_test *c) { (void)c; }

static Bool wait_for_notify(Display *dpy, XEvent *xev, XPointer arg)
{
	(void)dpy;
	return xev->type == MapNotify && xev->xmap.window == *(Window *)arg;
}

static Window create_window(
	Display *dpy,
	XVisualInfo *vi,
	const char *name)
{
	XSetWindowAttributes swa;
	Window root, w;
	XEvent xev;

	root = RootWindow(dpy, vi->screen);
	swa.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;

	w = XCreateWindow(
		dpy,
		root,		/* parent */
		0, 0,		/* (x, y) - usually ignored */
		WIDTH, HEIGHT,	/* width, height */
		0,		/* border width */
		vi->depth,
		InputOutput,	/* class */
		vi->visual,
		CWBorderPixel | CWColormap | CWEventMask,
		&swa);

	XStoreName(dpy, w, name);
	XMapWindow(dpy, w);
	XIfEvent(dpy, &xev, wait_for_notify, (XPointer)&w);

	return w;
}

static int destroy_window(Display *display, Window window)
{
	XWindowAttributes wa;

	(void)XGetWindowAttributes(display, window, &wa);
	(void)XFreeColormap(display, wa.colormap);
	return XDestroyWindow(display, window);
}

static Pixmap create_pixmap(Display *dpy, XVisualInfo *vi)
{
	Pixmap pm;
	Window root;

	root = RootWindow(dpy, vi->screen);
	pm = XCreatePixmap(dpy, root, WIDTH, HEIGHT, vi->depth);
	XFlush(dpy);
	return pm;
}

void gl_test_free(struct gl_test *test)
{
	glx_term_context(&test->ctx);
	test->free_xres(test->display, test->xres);
	XCloseDisplay(test->display);
	free(test);
}

struct gl_test *gl_test_make(char const *name)
{
	Display *display;
	XVisualInfo *vi;
	struct gl_test *test;
	struct glx_context *ctx;
	struct glx_drawable *drawable;

	test = malloc(sizeof *test);
	if (!test) { return NULL; }

	display = XOpenDisplay(NULL);
	if (!display) {
		free(test);
		return NULL;
	}

	ctx = &test->ctx;
	if (!glx_init_context(ctx, display, NULL)) {
		XCloseDisplay(display);
		free(test);
		return NULL;
	}

	vi = glx_visual_info(ctx);
	if (!vi) {
		glx_term_context(ctx);
		XCloseDisplay(display);
		free(test);
		return NULL;
	}

	test->interactive = name != NULL;
	if (name) {
		Window w = create_window(display, vi, name);
		drawable = glx_make_drawable_window(ctx, w);
		test->xres = w;
		test->free_xres = &destroy_window;
		test->swap_buffers = &swap_buffers;
	} else {
		Pixmap pm = create_pixmap(display, vi);
		drawable = glx_make_drawable_pixmap(ctx, pm);
		test->xres = pm;
		test->free_xres = &XFreePixmap;
		test->swap_buffers = &dont_swap_buffers;
	}
	test->drawable = drawable;
	test->display = display;
	XFree(vi);

	if (glx_make_current(ctx, drawable)) {
		gl_test_free(test);
		return NULL;
	}
	return test;
}

void gl_test_swap_buffers(struct gl_test *test)
{
	assert(test != NULL);
	test->swap_buffers(test);
}

void gl_test_wait_for_key(struct gl_test *test)
{
	XEvent xev;
	assert(test != NULL);
	if (!test->interactive) {
		return;
	}
	do {
		XNextEvent(test->display, &xev);
	} while (xev.type != KeyPress);
}

int gl_test_poll_key(struct gl_test *test)
{
	XEvent xev;
	assert(test != NULL);

	if (!test->interactive) {
		return 1;
	}

	while (XPending(test->display) > 0) {
		XNextEvent(test->display, &xev);
		if (xev.type == KeyPress) {
			return 1;
		}
	}
	return 0;
}
