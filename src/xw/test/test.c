
#include <stdio.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <ok/ok.h>

#include "../include/xw.h"
#include "../include/delegate.h"

#define TEXT_BUFFER_SIZE 500

struct context
{
	Display *display;
	Window window;
	bool print:1;
	bool destroy_on_close:1, is_closed:1;
	bool is_created:1, is_mapped:1, is_destroyed:1;
	bool has_focus:1, is_visible:1;
	char text[TEXT_BUFFER_SIZE], *cursor;
};

static struct context make_context(Display *display)
{
	struct context ctx;

	ctx.display = display;
	ctx.window = None;
	ctx.print = false;
	ctx.is_created = false;
	ctx.is_destroyed = false;
	ctx.is_mapped = false;
	ctx.destroy_on_close = false;
	ctx.is_closed = false;
	ctx.has_focus = false;
	ctx.is_visible = false;
	ctx.cursor = ctx.text;

	return ctx;
}

static void check_(int crit, int cond, char const *func, char const *message)
{
	if (!cond) {
		if (crit) {
			fail_test("%s %s\n", func, message);
		} else {
			(void)printf("%s %s\n", func, message);
			ok = -1;
		}
	}
}

#define check(cond, msg) \
	check_(0, cond, __func__, "failed \"" #cond "\": " msg)

#define critical_check(cond, msg) \
	check_(1, cond, __func__, "failed \"" #cond "\": " msg)

static void escape(char const *str, size_t n, FILE *fp)
{
	size_t i;
	for (i = 0; i < n; i++) {
		switch (str[i]) {
		case '\\': (void)fprintf(fp, "\\\\"); break;
		case '\a': (void)fprintf(fp, "\\a"); break;
		case '\b': (void)fprintf(fp, "\\b"); break;
		case '\f': (void)fprintf(fp, "\\f"); break;
		case '\n': (void)fprintf(fp, "\\n"); break;
		case '\r': (void)fprintf(fp, "\\r"); break;
		case '\t': (void)fprintf(fp, "\\t"); break;
		case '\v': (void)fprintf(fp, "\\v"); break;
		case '"': (void)fprintf(fp, "\\\""); break;
		default: (void)fputc(str[i], fp); break;
		}
	}
}

static char const *bool_name(int value)
{
	return value ? "true" : "false";
}

static int (*previous_error_handler)(Display *, XErrorEvent *);
static int (*previous_io_error_handler)(Display *);

static int error_handler(Display *display, XErrorEvent *error_event)
{
	static char msg[1000], req[1000], min[1000]; 
	(void)XGetErrorText(display, error_event->error_code, msg, sizeof msg);
	(void)XCloseDisplay(display);
	(void)XSetErrorHandler(previous_error_handler);
	(void)XSetIOErrorHandler(previous_io_error_handler);
	fail_test("X11 Error %s (%s, %s)\n", msg, req, min);
	return -1;
}

static int io_error_handler(Display *display)
{
	(void)XCloseDisplay(display);
	(void)XSetErrorHandler(previous_error_handler);
	(void)XSetIOErrorHandler(previous_io_error_handler);
	fail_test("X11 IO Error\n");
	return -1;
}

static Display *open_display(void)
{
	Display *display = XOpenDisplay(NULL);
	if (!display) skip_test("Unable to open display");
	return display;
}

static int test_common(int test(struct xw_state *state, Display *display))
{
	int result;
	Display *display;
	struct xw_state *state;

	/* Restore default error handlers */
	previous_error_handler = XSetErrorHandler(error_handler);
	previous_io_error_handler = XSetIOErrorHandler(io_error_handler);

	display = open_display();
	state = xw_make(display, NULL, NULL);
	critical_check(state != NULL, "xw_make()");
	
	result = test(state, display);

	check(xw_free(state) == 0, "xw_free()");
	(void)XCloseDisplay(display);

	/* Restore default error handlers */
	(void)XSetErrorHandler(previous_error_handler);
	(void)XSetIOErrorHandler(previous_io_error_handler);

	return ok || result;
}

static void on_create(Window window, void *context)
{
	struct context *ctx = context;

	check(ctx->is_created == false,
		"window is not created before `create`");

	check(ctx->window == None,
		"no window reference before calling `create`");

	ctx->window = window;
	ctx->is_created = true;
	ctx->is_destroyed = false;

	if (ctx->print) {
		(void)fprintf(stderr,
			"create(window = %lu, context = %p)\n",
			(unsigned long)window,
			context);
	}
}

static void on_map(void *context)
{
	struct context *ctx = context;

	check(ctx->is_mapped == false,
		"window is mapped only once before being unmapped");

	ctx->is_mapped = true;

	if (ctx->print) {
		(void)fprintf(stderr, "map(context = %p)\n", context);
	}
}

void on_close(void *context)
{
	struct context *ctx = context;

	ctx->is_closed = true;
	if (ctx->destroy_on_close) {
		(void)XDestroyWindow(ctx->display, ctx->window);
	}
	if (ctx->print) {
		(void)fprintf(stderr, "close(context = %p)\n", context);
	}
}

static void on_unmap(void *context)
{
	struct context *ctx = context;

	check(ctx->is_mapped == true,
		"window is mapped before being unmapped");

	ctx->is_mapped = false;

	if (ctx->print) {
		(void)fprintf(stderr, "unmap(context = %p)\n", context);
	}
}

static void on_destroy(void *context)
{
	struct context *ctx = context;

	check(ctx->is_created == true,
		"window is created before being destroyed");

	check(ctx->is_destroyed == false,
		"window is destroyed only after it has been created");

	ctx->is_created = false;
	ctx->is_destroyed = true;
	ctx->window = None;

	if (ctx->print) {
		(void)fprintf(stderr, "destroy(context = %p)\n", context);
	}
}

static void on_text(char const *utf8, size_t size, void *context)
{
	struct context *ctx = context;
	if (ctx->print) {
		(void)fprintf(stderr, "text(utf8 = \"");
		escape(utf8, size, stderr);
		(void)fprintf(stderr, "\", size = %zu, ", size);
		(void)fprintf(stderr, "context = %p)\n", context);
	}
}

static void on_key_press(KeySym sym, int state, int is_repeat, void *context)
{
	struct context *ctx = context;
	if (ctx->print) {
		(void)fprintf(
			stderr,
			"key_press(sym = %s, state = 0x%x, is_repeat = %s, context = %p)\n",
			XKeysymToString(sym),
			(unsigned)state,
			bool_name(is_repeat),
			context);
	}
}

static void on_key_release(KeySym sym, int state, void *context)
{
	struct context *ctx = context;
	if (ctx->print) {
		(void)fprintf(
			stderr,
			"key_release(sym = %s, state = 0x%x, context = %p)\n",
			XKeysymToString(sym),
			(unsigned)state,
			context);
	}
}

void on_visibility(int is_visible, void *context)
{
	struct context *ctx = context;
	ctx->is_visible = is_visible;
	if (ctx->print) {
		(void)fprintf(
			stderr,
			"visibility(is_visible = %s, context = %p)\n",
			bool_name(is_visible),
			context);
	}
}

void on_focus(int has_focus, void *context)
{
	struct context *ctx = context;
	ctx->has_focus = has_focus;
	if (ctx->print) {
		(void)fprintf(
			stderr,
			"focus(has_focus = %s, context = %p)\n",
			bool_name(has_focus),
			context);
	}
}

static struct xw_delegate const all_events = {
	.create = on_create,
	.map = on_map,
	.close = on_close,
	.unmap = on_unmap,
	.destroy = on_destroy,
	.text = on_text,
	.key_press = on_key_press,
	.key_release = on_key_release,
	.focus = on_focus,
	.visibility = on_visibility
};

static void send_close_request(Display *display, Window window)
{
	XEvent buf;
	XClientMessageEvent *ev = &buf.xclient;

	ev->type = ClientMessage;
	ev->display = display;
	ev->window = window;
	ev->message_type = XInternAtom(display, "WM_PROTOCOLS", False);
	ev->format = 32;
	ev->data.l[0] = (long)XInternAtom(display, "WM_DELETE_WINDOW", False);
	ev->data.l[1] = CurrentTime;

	if (!XSendEvent(display, window, False, 0, &buf)) {
		fail_test(__func__);
	}
}

static int create_window_(struct xw_state *state, Display *display)
{
	static struct xw_delegate const delegate = {
		.create = on_create,
		.map = on_map,
		.unmap = on_unmap,
		.destroy = on_destroy
	};

	struct context ctx = make_context(display);
	if(xw_create_window(state, &delegate, &ctx, 0, __func__, 600, 600)) {
		fail_test("Failed to open window");
	}
	check(ctx.is_created, "`create` is called");

	/* Wait until `map` is called (or wait forever) */
	while (!ctx.is_mapped) { xw_handle_events(state); }

	(void)XDestroyWindow(display, ctx.window);
	while (!ctx.is_destroyed) { xw_handle_events(state); }

	check(ctx.is_mapped == false, "`unmap` is called");
	check(ctx.is_destroyed, "`destroy` is called");

	return ok;
}

static int create_window(void)
{
	return test_common(create_window_);
}

static int close_window_(struct xw_state *state, Display *display)
{
	static struct xw_delegate const delegate = {
		.create = on_create,
		.map = on_map,
		.close = on_close,
		.unmap = on_unmap,
		.destroy = on_destroy
	};

	struct context ctx = make_context(display);
	ctx.destroy_on_close = true;

	if(xw_create_window(state, &delegate, &ctx, 0, __func__, 600, 600)) {
		fail_test("Failed to open window");
	}
	/* Wait until `map` is called (or wait forever) */
	while (!ctx.is_mapped) { xw_handle_events(state); }

	send_close_request(display, ctx.window);

	while (!ctx.is_destroyed) {
		xw_handle_events(state);
	}
	check(ctx.is_closed == true, "`close` is called");
	check(ctx.is_mapped == false, "`unmap` is called");
	check(ctx.is_destroyed, "`destroy` is called");

	return ok;

}

static int close_window(void)
{
	return test_common(close_window_);
}

static int interactive_(struct xw_state *state, Display *display)
{
	struct context ctx = make_context(display);
	ctx.print = true;
	ctx.destroy_on_close = true;
	if(xw_create_window(state, &all_events, &ctx, 0, __func__, 600, 600)) {
		fail_test("Failed to open window");
	}
	while (!ctx.is_destroyed) {
		xw_handle_events(state);
	}
	return ok;
}

static int interactive(void)
{
	return test_common(interactive_);
}

struct test const tests[] = {
	{ create_window, "create a window" },
	{ close_window, "close a window through WM messages" },
	{ interactive, "key presses" },

	{ NULL, NULL }
};

