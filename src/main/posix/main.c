#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <xw/xw.h>
#include <xw/delegate.h>
#include <glapi/x.h>
#include <glapi/xtypes.h>

#include "window.h"

#include <text/token.h>

sig_atomic_t signum;
sig_atomic_t has_signal = 0;

static void on_signal(int s)
{
	signum = s;
	has_signal = 1;
}

static void die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}

static void die_perror(const char *func)
{
	die("%s: %s\n", func, strerror(errno));
}

int main(void)
{
	struct xw_state *xw;
	struct glx_context *ctx;
	Display *display;
	int xfd, err, done;
	fd_set readfds, writefds, exceptfds;
	struct timeval tv;
	struct window *window;
	struct sigaction sa;

	sa.sa_handler = on_signal;
	if (sigemptyset(&sa.sa_mask)) { die_perror("sigemptyset"); }
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL)) {
		die("sigaction: %s\n", strerror(errno));
	}
	if (sigaction(SIGQUIT, &sa, NULL)) {
		die("sigaction: %s\n", strerror(errno));
	}
	if (display = XOpenDisplay(NULL), !display) {
		die("XOpenDisplay");
	}
	if (xw = xw_make(display, NULL, NULL), !xw) {
		die("Failed to initialize window module!\n");
	}
	if (ctx = glx_make_context(display, NULL), !ctx) {
		die("Failed to initialize OpenGL module!\n");
	}
	if (window = create_window(display, xw, ctx), !window) {
		die("Failed to create main window!\n");
	}
	done = 0;
	xfd = ConnectionNumber(display);
	(void)XFlush(display);
	while (!done) {
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		/* Read X11 messages from display socket */
		FD_SET(xfd, &readfds);

		/* Read commands from STDIN */
		FD_SET(STDIN_FILENO, &readfds);

		tv.tv_sec = 0;
		tv.tv_usec = 10;
		err = select(xfd + 1, &readfds, &writefds, &exceptfds, &tv);
		if (err < 0) {
			if (errno == EINTR) {
				done = 1;
			} else {
				die("select: %s\n", strerror(errno));
			}
		}
		if (has_signal) {
			done = 1;
		}
		if (FD_ISSET(xfd, &readfds)) {
			xw_handle_events(xw);
		}
		if (FD_ISSET(STDIN_FILENO, &readfds)) {
			int n;
			char line[500];

			n = tokenize_line(line, sizeof line, '\0', stdin);
			if (n > 0) {
				if (strcmp(line, "quit") == 0) {
					done = 1;
				}
				else {
					(void)printf("what is ``%s''?\n", line);
				}
			}
		}
		if (window->drawable) {
			glx_make_current(ctx, window->drawable);

			/* Render */
			glClearColor(1.f, 1.f, 1.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glx_swap_buffers(ctx, window->drawable);
		}
	}
	glx_free_context(ctx);
	if (xw_free(xw)) {
		(void)fprintf(stderr, "Failed to free window module!\n");
	}
	(void)XCloseDisplay(display);

	return 0;
}
