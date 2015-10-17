
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#define WIDTH 600
#define HEIGHT WIDTH

#include "../types.h"
#include "../include/xtypes.h"
#include "../decl.h"
#include "../include/x.h"

#include "xdrawable.h"

struct gltest
{
	Display *display;
	XID xres;
	struct glxdrawable *drawable;
	struct glxstate xstate;
	int (*free_xres)(Display *, XID);
	void (*swap_buffers)(struct gltest *);
};

struct glstate *gl_test_get_state(struct gltest *test)
{
	assert(test);
	return &test->xstate.state;
}

static void swap_buffers(struct gltest *test)
{
	glXSwapBuffers(test->display, test->drawable->glxid);
}

static void dont_swap_buffers(struct gltest *c) { (void)c; }

static Bool wait_for_notify(Display *dpy, XEvent *xev, XPointer arg)
{
	(void)dpy;
	return xev->type == MapNotify && xev->xmap.window == *(Window *)arg;
}

static Bool wait_for_expose(Display *dpy, XEvent *xev, XPointer arg)
{
	(void)dpy;
	return xev->type == Expose && xev->xexpose.window == *(Window *)arg;
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
	XIfEvent(dpy, &xev, wait_for_expose, (XPointer)&w);

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

void gl_free_test_context(struct gltest *test)
{
	gl_free_xcontext(&test->xstate);
	test->free_xres(test->display, test->xres);
	XCloseDisplay(test->display);
	free(test);
}

struct gltest *gl_make_test_context(char const *name)
{
	Display *display;
	XVisualInfo *vi;
	struct gltest *test;
	struct glxstate *xstate;
	struct glxdrawable *drawable;

	test = malloc(sizeof *test);
	if (!test) { return NULL; }

	display = XOpenDisplay(NULL);
	if (!display) {
		free(test);
		return NULL;
	}

	xstate = &test->xstate;
	if (!gl_make_xcontext(xstate, display, NULL)) {
		XCloseDisplay(display);
		free(test);
		return NULL;
	}

	vi = gl_visual_info(xstate);
	if (!vi) {
		gl_free_xcontext(xstate);
		XCloseDisplay(display);
		free(test);
		return NULL;
	}

	if (name) {
		Window w = create_window(display, vi, name);
		drawable = gl_add_xwindow(xstate, w);
		test->xres = w;
		test->free_xres = &destroy_window;
		test->swap_buffers = &swap_buffers;
	} else {
		Pixmap pm = create_pixmap(display, vi);
		drawable = gl_add_xpixmap(xstate, pm);
		test->xres = pm;
		test->free_xres = &XFreePixmap;
		test->swap_buffers = &dont_swap_buffers;
	}
	test->drawable = drawable;
	test->display = display;
	XFree(vi);

	if (gl_make_current(xstate, drawable)) {
		gl_free_test_context(test);
		return NULL;
	}
	return test;
}

void gl_test_swap_buffers(struct gltest *test)
{
	assert(test != NULL);
	test->swap_buffers(test);
}

void gl_test_wait_for_key(struct gltest *test)
{
	XEvent xev;
	assert(test != NULL);
	do {
		XNextEvent(test->display, &xev);
	} while (xev.type != KeyPress);
}

