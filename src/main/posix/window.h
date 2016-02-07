
struct window {
	Display *display;
	Window window;
	struct xw_state *xw;
	struct glx *glx;
	struct glx_drawable *drawable;
};

struct window *create_window(
	Display *display,
	struct xw_state *xw,
	struct glx *glx);

void destroy_window(struct window *);

