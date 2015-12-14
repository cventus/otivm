
struct window {
	Display *display;
	Window window;
	struct xw_state *xw;
	struct gl_xstate *gl;
	struct gl_xdrawable *drawable;
};

struct window *create_window(
	Display *display,
	struct xw_state *xw,
	struct gl_xstate *gl);

void destroy_window(struct window *);

