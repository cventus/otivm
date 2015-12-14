
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <assert.h>

#include <opengl/x.h>
#include <xw/xw.h>
#include <xw/delegate.h>
#include <base/mem.h>

#include "window.h"

static void create(Window window, void *context);
static void map(void *context);
static void resize(int width, int height, void *context);
static void destroy(void *context);

static struct xw_delegate const delegate = {
	.create = create,
	.map = map,
	.resize = resize,
	.destroy = destroy
};

struct window *create_window(
	Display *display,
	struct xw_state *xw,
	struct gl_xstate *gl)
{
	int err;
	struct window *window;
	XVisualInfo *vi;

	if (vi = gl_visual_info(gl), !vi) {
		return NULL;
	}
	if (window = malloc(sizeof *window), window == NULL) {
		(void)XFree(vi);
		return NULL;
	}
	window->display = display;
	window->xw = xw;
	window->gl = gl;
	window->window = None;
	window->drawable = NULL;
	err = xw_create_window(
		xw, &delegate, window, vi, "Hello, world", 800, 600);
	(void)XFree(vi);
	if (err) {
		free(window);
		return NULL;
	}
	return window;
}

void destroy_window(struct window *window)
{
	(void)XDestroyWindow(window->display, window->window);
}

static void create(Window window, void *context)
{
	struct window *w = context;
	w->window = window;
}

static void map(void *context)
{
	struct window *window = context;
	
	if (window->drawable == NULL) {
		window->drawable = gl_add_xwindow(window->gl, window->window);
		if (window->drawable == NULL) {
			(void)fprintf(
				stderr,
				"Failed to initialize OpenGL for window!\n");
			destroy_window(window);
		}
	}
}

static void resize(int width, int height, void *context)
{
	(void)width;
	(void)height;
	(void)context;
}

static void destroy(void *context)
{
	free(context);
}

