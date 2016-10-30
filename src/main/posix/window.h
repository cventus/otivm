
struct window {
	Display *display;
	Window window;
	struct xw_state *xw;
	struct glx_context *ctx;
	struct glx_drawable *drawable;
};

struct window *create_window(
	Display *display,
	struct xw_state *xw,
	struct glx_context *glx);

void destroy_window(struct window *);

