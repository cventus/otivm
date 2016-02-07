
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>

#include <base/mem.h>
#include <base/wbuf.h>

#include "include/xw.h"
#include "include/delegate.h"

#define BASIC_XIM_STYLES (XIMPreeditNothing | XIMStatusNothing )

struct xw_state
{
	Display *display;
	XIM im;

	char const *res_name;
	char const *app_name;
	Atom wm_delete_window;

	/* Array of pointers to struct window */
	struct wbuf windows;
};

struct window
{
	XIC ic;
	struct xw_delegate const *delegate;
	void *context;
	Window window;
	int is_open, is_visible, width, height;
	unsigned char key_state[UCHAR_MAX >> 3];
};

static long create_event_mask(struct xw_delegate const *delegate)
{
	long mask = StructureNotifyMask;

	if (delegate->visibility) { mask |= VisibilityChangeMask; }
	if (delegate->focus) { mask |= FocusChangeMask; }
	if (delegate->key_press || delegate->text) { mask |= KeyPressMask; }
	if (delegate->key_release) { mask |= KeyReleaseMask; }
	if (delegate->button_press) { mask |= ButtonPressMask; }
	if (delegate->button_release) { mask |= ButtonReleaseMask; }
	if (delegate->pointer_motion) { mask |= PointerMotionMask; }
	if (delegate->pointer_enter) { mask |= EnterWindowMask; }
	if (delegate->pointer_leave) { mask |= LeaveWindowMask; }

	return mask;
}

static Window create_window(
	Display *display,
	XVisualInfo *vi,
	unsigned int width,
	unsigned int height,
	long event_mask)
{
	XSetWindowAttributes swa;

	int screen = vi ? vi->screen : DefaultScreen(display);
	int depth = vi ? vi->depth : DefaultDepth(display, screen);
	Visual *visual = vi ? vi->visual : DefaultVisual(display, screen);

	Window root = RootWindow(display, screen);
	swa.colormap = XCreateColormap(display, root, visual, AllocNone);
	swa.event_mask = event_mask;

	return XCreateWindow(
		display,
		root,
		0,
		0,
		width,
		height,
		0,
		depth,
		InputOutput, 
		visual, 
		CWColormap | CWEventMask,
		&swa);
}

static XIC make_ic(struct xw_state *state, Window window)
{
	XIC ic;

	ic = XCreateIC(state->im,
		XNInputStyle, BASIC_XIM_STYLES,
		XNClientWindow, window,
		NULL);

	return ic;
}

int xw_create_window(
	struct xw_state *state,
	struct xw_delegate const *delegate,
	void *context,
	XVisualInfo *vi,
	char const *title,
	unsigned width,
	unsigned height)
{
	Status status;
	Window w;
	struct window *window, **p;
	long event_mask;
	Display *display;

	assert(state != NULL);
	assert(state->display != NULL);
	assert(delegate != NULL);

	display = state->display;
	p = wbuf_alloc(&state->windows, sizeof *p);
	if (!p) { return -1; }

	window = malloc(sizeof *window);
	if (!window) { goto err0; }
	
	event_mask = create_event_mask(delegate);
	w = create_window(display, vi, width, height, event_mask);

	/* Handle close-window event */
	status = XSetWMProtocols(display, w, &state->wm_delete_window, 1);
	if (status == 0) {
		(void)fprintf(stderr, "WM_DELETE_WINDOW not suppoted\n");
	}
	(void)XStoreName(display, w, title);
	/* TODO: add WM_CLASS property to window */

	window->width = window->height = 0;
	window->window = w;
	window->delegate = delegate;
	window->context = context;
	window->is_open = 0;
	window->ic = make_ic(state, w);
	memset(window->key_state, 0, sizeof window->key_state);
	*p = window;
	if (delegate->create) {
		delegate->create(w, context);
	}
	(void)XMapWindow(display, w);
	return 0;

err0:	(void)wbuf_retract(&state->windows, sizeof *p);
	return -1;
}

static void xw_destroy_window(struct xw_state *state, struct window *w)
{
	w->is_open = 0;
	(void)XDestroyWindow(state->display, w->window);
}

static bool supports_style(XIMStyles *styles, XIMStyle desiredStyle)
{
	unsigned short i;
	XIMStyle style;
	for (i = 0; i < styles->count_styles; i++) {
		style = styles->supported_styles[i];
		if ((style & desiredStyle) == style) {
			return true;
		}
	}
	return false;
}

static XIM make_im(Display *display)
{
	XIM im;
	XIMStyles *styles;
	char *old_mods, *param;
	bool match;

	if (!XSupportsLocale()) { return NULL; }

	old_mods = XSetLocaleModifiers("@im=none");
	if (old_mods == NULL) { return NULL; }

	if ((im = XOpenIM(display, NULL, NULL, NULL))) {
		match = false;
		param = XGetIMValues(im, XNQueryInputStyle, &styles, NULL);
		if (param) {
			(void)fprintf(stderr,
				"%s: failed to get IM value: %s\n",
				__func__, param);
		} else {
			match = supports_style(styles, BASIC_XIM_STYLES);
			XFree(styles);
		}
		if (!match) {
			(void)XCloseIM(im);
			im = NULL;
		}
	}
	(void)XSetLocaleModifiers(old_mods);
	return im;
}

struct xw_state *xw_make(
	Display *display,
	//char const *display_name,
	char const *res,
	char const *app)
{
	struct xw_state *state;
	size_t rsz, asz;

	assert(display != NULL);

	if (!XkbSetDetectableAutoRepeat(display, True, NULL)) {
		(void)fprintf(stderr, "Detectable auto repeat not set!");
	}

	/* FIXME: `res` can be provided by user -- potential for overflow */
	rsz = res ? strlen(res) + 1 : 0;
	asz = app ? strlen(app) + 1 : 0;
	state = malloc(sizeof *state + rsz + asz);
	if (!state) { return NULL; }
	state->im = make_im(display);
	state->res_name = res ? strcpy((char *)(state + 1), res) : NULL;
	state->app_name = app ? strcpy((char *)(state + 1) + rsz, app) : NULL;
	state->display = display;
	wbuf_init(&state->windows);
	state->wm_delete_window =
		XInternAtom(display, "WM_DELETE_WINDOW", False);

	return state;
}

int xw_free(struct xw_state *state)
{
	struct window **w, **end;

	for (w = state->windows.begin, end = state->windows.end; w < end; w++) {
		xw_destroy_window(state, *w);
	}
	XFlush(state->display);
	xw_handle_events(state);
	if (state->im) {
		(void)XCloseIM(state->im);
		state->im = NULL;
	}
	state->display = NULL;
	wbuf_term(&state->windows);
	free(state);
	return 0;
}

static struct window **get_window_loc(struct xw_state *state, Window window)
{
	struct window **p, **end;

	for (p = state->windows.begin, end = state->windows.end; p < end; p++) {
		if ((*p)->window == window) {
			return p;
		}
	}
	return NULL;
}

static struct window *get_window(struct xw_state *state, Window window)
{
	struct window **p = get_window_loc(state, window);
	return p ? *p : NULL;
}

static bool set_key_state(unsigned char *key_state, unsigned char keycode)
{
	unsigned idx = keycode >> 3, bit = keycode & 7;
	bool prev = key_state[idx] & (1 << bit);
	key_state[idx] |= (1 << bit);
	return prev;
}

static bool clear_key_state(unsigned char *key_state, unsigned char keycode)
{
	unsigned idx = keycode >> 3, bit = keycode & 7;
	bool prev = key_state[idx] & (1 << bit);
	key_state[idx] &= ~(1 << bit);
	return prev;
}

static void on_keypress(struct xw_state *state, XEvent *event)
{
	XKeyPressedEvent *e = &event->xkey;
	struct window *w = get_window(state, e->window);

	if (!w || !(w->delegate->key_press || w->delegate->text)) { return; }

	if (w->delegate->key_press) {
		KeySym sym = XLookupKeysym(&event->xkey, 0);
		if (sym != NoSymbol) {
			w->delegate->key_press(
				sym,
				e->state,
				set_key_state(w->key_state, e->keycode),
				w->context);
		}
	}
	if (w->delegate->text && !XFilterEvent(event, None)) {
		Status status;
		char buffer[64], *p = buffer;
		int size = sizeof buffer, n;

again:		n = Xutf8LookupString(w->ic, e, p, size, NULL, &status);
		if (status == XBufferOverflow) {
			size = n + 1;
			if (p != buffer) { free(p); }
			p = malloc(n);
			goto again;
		}
		if (status == XLookupChars || status == XLookupBoth)
		if (n > 0 && n < size) {
			p[n] = '\0';
			w->delegate->text(p, n, w->context);
		}
		if (p != buffer) { free(p); }
	}
}

static void on_keyrelease(struct xw_state *state, XEvent *event)
{
	XKeyReleasedEvent const *e = &event->xkey;
	struct window *w = get_window(state, e->window);
	if (!w || !w->delegate->key_release) { return; }

	if (clear_key_state(w->key_state, e->keycode)) {
		KeySym sym = XLookupKeysym(&event->xkey, 0);
		w->delegate->key_release(sym, e->state, w->context);
	}
}

static void on_buttonpress(struct xw_state *state, XEvent *event)
{
	XButtonPressedEvent const *e = &event->xbutton;
	struct window *w = get_window(state, e->window);
	if (w && w->delegate->button_press) {
		w->delegate->button_press(
			e->button,
			e->state,
			e->x,
			e->y,
			w->context);
	}
}

static void on_buttonrelease(struct xw_state *state, XEvent *event)
{
	XButtonReleasedEvent const *e = &event->xbutton;
	struct window *w = get_window(state, e->window);
	if (w && w->delegate->button_release) {
		w->delegate->button_release(
			e->button,
			e->state,
			e->x,
			e->y,
			w->context);
	}
}

static void on_motion(struct xw_state *state, XEvent *event)
{
	XMotionEvent const *e = &event->xmotion;
	struct window *w = get_window(state, e->window);
	if (w && w->delegate->pointer_motion) {
		w->delegate->pointer_motion(
			e->state,
			e->x,
			e->y,
			w->context);
	}
}

static void on_enter(struct xw_state *state, XEvent *event)
{
	XEnterWindowEvent const *e = &event->xcrossing;
	struct window *w = get_window(state, e->window);
	if (w && w->delegate->pointer_enter) {
		w->delegate->pointer_enter(e->state, e->x, e->y, w->context);
	}
}

static void on_leave(struct xw_state *state, XEvent *event)
{
	XLeaveWindowEvent const *e = &event->xcrossing;
	struct window *w = get_window(state, e->window);
	if (w && w->delegate->pointer_leave) {
		w->delegate->pointer_leave(e->state, e->x, e->y, w->context);
	}
}

static void on_focusin(struct xw_state *state, XEvent *event)
{
	XFocusInEvent const *e = &event->xfocus;
	struct window *w = get_window(state, e->window);
	if (w && w->delegate->focus) {
		w->delegate->focus(1, w->context);
	}
}

static void on_focusout(struct xw_state *state, XEvent *event)
{
	XFocusOutEvent const *e = &event->xfocus;
	struct window *w = get_window(state, e->window);
	if (w && w->delegate->focus) {
		w->delegate->focus(0, w->context);
	}
}

static void on_visibility(struct xw_state *state, XEvent *event)
{
	XVisibilityEvent const *e = &event->xvisibility;
	struct window *w = get_window(state, e->window);
	int is_visible =
		e->state == VisibilityUnobscured ||
		e->state == VisibilityPartiallyObscured;
	if (w && w->is_visible != is_visible) {
		w->is_visible = is_visible;
		if (w->delegate->visibility) {
			w->delegate->visibility(is_visible, w->context);
		}
	}
}

static void on_destroy(struct xw_state *state, XEvent *event)
{
	XDestroyWindowEvent const *e = &event->xdestroywindow;
	struct window **p = get_window_loc(state, e->window), *w;
	if (p) {
		w = *p;
		/* We shouldn't store null-pointers in the window array */
		assert(w != NULL);
		if (w->delegate->destroy) {
			w->delegate->destroy(w->context);
		}
		/* Only now remove the window from the list */
		(void)wbuf_pop(&state->windows, p, sizeof *p);
		free(w);
	} else {
		(void)fprintf(stderr, "Unknown window was destroyed!\n");
	}
}

static void on_unmap(struct xw_state *state, XEvent *event)
{
	XMapEvent const *e = &event->xmap;
	struct window *w = get_window(state, e->window);
	if (w) {
		w->is_open = 0;
		if (w->delegate->unmap) {
			w->delegate->unmap(w->context);
		}
	}
}

static void on_map(struct xw_state *state, XEvent *event)
{
	XMapEvent const *e = &event->xmap;
	struct window *w = get_window(state, e->window);
	if (w) {
		w->is_open = 1;
		if (w->delegate->map) {
			w->delegate->map(w->context);
		}
	}
}

static void on_configure(struct xw_state *state, XEvent *event)
{
	XConfigureEvent const *e = &event->xconfigure;
	struct window *w = get_window(state, e->window);
	if (w && (w->width != e->width || w->height != e->height)) {
		w->width = e->width;
		w->height = e->height;
		if (w->delegate->resize) {
			w->delegate->resize(e->width, e->height, w->context);
		}
	}
}

static void on_clientmessage(struct xw_state *state, XEvent *event)
{
	XClientMessageEvent const *e = &event->xclient;

	if (e->format == 32 && (Atom)e->data.l[0] == state->wm_delete_window) {
		/* WM_DELETE_WINDOW */
		struct window *w = get_window(state, e->window);
		if (w && w->delegate->close) {
			w->delegate->close(w->context);
		} else {
			/* default behavior */
			xw_destroy_window(state, w);
		}
	}
}

static void (* const handlers[])(struct xw_state *state, XEvent *ev) = {
	/* keyboard */
	[KeyPress] = on_keypress,
	[KeyRelease] = on_keyrelease,

	/* mouse */
	[ButtonPress] = on_buttonpress,
	[ButtonRelease] = on_buttonrelease,
	[MotionNotify] = on_motion,
	[EnterNotify] = on_enter,
	[LeaveNotify] = on_leave,

	/* focus */
	[FocusIn] = on_focusin,
	[FocusOut] = on_focusout,

	/* visibility */
	[VisibilityNotify] = on_visibility,

	/* window life-time */
	[DestroyNotify] = on_destroy,
	[MapNotify] = on_map,
	[UnmapNotify] = on_unmap,

	/* Move/resize, etc. */
	[ConfigureNotify] = on_configure,

	/* misc */
	[ClientMessage] = on_clientmessage,
};

int xw_handle_events(struct xw_state *state)
{
	while (XPending(state->display) > 0) {
		XEvent e;
		XNextEvent(state->display, &e);
		if ((size_t)e.type < length_of(handlers) && handlers[e.type]) {
			handlers[e.type](state, &e);
		}
	}
	return 0;
}

