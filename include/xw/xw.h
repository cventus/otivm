
struct xw_state;
struct xw_delegate;

struct xw_state *xw_make(
	Display *display,
	char const *res_name,
	char const *app_name);

int xw_free(struct xw_state *state);

int xw_create_window(
	struct xw_state *state,
	struct xw_delegate const *d,
	void *context,
	XVisualInfo *vi,
	char const *title,
	unsigned width,
	unsigned height);

int xw_enable_text(struct xw_state *state, Window window, int enabled);
int xw_is_text_enabled(struct xw_state *state, Window window);

struct xw_delegate *xw_get_delegate(struct xw_state *state, Window window);

int xw_handle_events(struct xw_state *state);

